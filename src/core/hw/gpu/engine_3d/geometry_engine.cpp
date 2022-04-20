#include <unordered_map>
#include "common/log.h"
#include "core/hw/gpu/engine_3d/geometry_engine.h"
#include "core/hw/gpu/gpu.h"
#include "core/core.h"

static constexpr std::array<int, 256> param_table = {{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 1, 1, 1, 0, 16, 12, 16, 12, 9, 3, 3, 0, 0, 0,
    1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
    1, 1, 1, 1, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
}};

GeometryEngine::GeometryEngine(GPU* gpu) : gpu(gpu) {}

void GeometryEngine::Reset() {
    gxstat = 0;
    gxfifo = 0;
    gxfifo_write_count = 0;
    vertex_ram_size = 0;
    vertex_count = 0;
    screen_x1 = screen_x2 = screen_y1 = screen_y2 = 0;

    std::queue<Entry> empty_fifo_queue;
    fifo.swap(empty_fifo_queue);

    std::queue<Entry> empty_pipe_queue;
    pipe.swap(empty_pipe_queue);

    state = GeometryEngineState::Running;

    busy = false;

    polygon_type = PolygonType::Triangle;
    matrix_mode = MatrixMode::Projection;
    polygon_ram_size = 0;
    disp_1dot_depth = 0;

    geometry_command_event = gpu->system.scheduler.RegisterEvent("GeometryCommand", [this]() {
        busy = false;
        InterpretCommand();
    });

    texture_parameters = 0;
}

u32 GeometryEngine::ReadGXSTAT() {
    u32 data = 0;

    data |= (fifo.size() << 16);

    if (fifo.size() == 256) {
        data |= (1 << 24);
    }

    if (fifo.size() < 128) {
        data |= (1 << 25);
    }

    if (fifo.size() == 0) {
        data |= (1 << 26);
    }

    u8 fifo_irq_mode = (gxstat >> 30) & 0x3;
    
    data |= (fifo_irq_mode << 30);

    return data;
}

void GeometryEngine::WriteGXSTAT(u32 data) {
    // TODO: handle side effects later
    gxstat = data;
}

void GeometryEngine::WriteGXFIFO(u32 data) {
    if (gxfifo == 0) {
        gxfifo = data;
    } else {
        u8 command = gxfifo & 0xFF;
        QueueEntry({command, data});

        gxfifo_write_count++;

        if (gxfifo_write_count == param_table[command]) {
            gxfifo >>= 8;
            gxfifo_write_count = 0;
        }
    }

    while ((gxfifo != 0) && (param_table[gxfifo & 0xFF] == 0)) {
        u8 command = gxfifo & 0xFF;
        QueueEntry({command, 0});
        gxfifo >>= 8;
    }
}

void GeometryEngine::QueueCommand(u32 addr, u32 data) {
    u8 command = (addr >> 2) & 0x7F;
    QueueEntry({command, data});
}

void GeometryEngine::QueueEntry(Entry entry) {
    if (fifo.size() == 0 && pipe.size() < 4) {
        pipe.push(entry);
    } else {
        fifo.push(entry);

        while (fifo.size() >= 256) {
            // just run commands until the fifo isn't full
            busy = false;
            gpu->system.scheduler.CancelEvent(&geometry_command_event);
            InterpretCommand();
        }
    }

    InterpretCommand();
}

Entry GeometryEngine::DequeueEntry() {
    Entry entry = pipe.front();
    pipe.pop();
    
    // if the pipe is running half empty
    // move 2 entries from the fifo to the pipe
    if (pipe.size() < 3) {
        if (fifo.size() > 0) {
            pipe.push(fifo.front());
            fifo.pop();
        }

        if (fifo.size() > 0) {
            pipe.push(fifo.front());
            fifo.pop();
        }

        CheckGXFIFOInterrupt();

        // TODO: do dma stuff
    }

    return entry;
}

void GeometryEngine::InterpretCommand() {
    int total_size = fifo.size() + pipe.size();

    if (busy || (total_size == 0)) {
        return;
    }

    u8 command = pipe.front().command;
    u8 param_count = param_table[command];

    if (total_size >= param_count) {
        switch (command) {
        case 0x10:
            SetMatrixMode();
            break;
        case 0x11:
            PushCurrentMatrix();
            break;
        case 0x12:
            PopCurrentMatrix();
            break;
        case 0x13:
            StoreCurrentMatrix();
            break;
        case 0x14:
            RestoreCurrentMatrix();
            break;
        case 0x15:
            LoadUnitMatrix();
            break;
        case 0x16:
            Load4x4();
            break;
        case 0x17:
            Load4x3();
            break;
        case 0x18:
            Multiply4x4();
            break;
        case 0x19:
            Multiply4x3();
            break;
        case 0x1A:
            Multiply3x3();
            break;
        case 0x1B:
            MultiplyScale();
            break;
        case 0x1C:
            MultiplyTranslation();
            break;
        case 0x20:
            SetVertexColour();
            break;
        case 0x21:
            SetNormalVector();
            break;
        case 0x22:
            SetTextureCoordinates();
            break;
        case 0x23:
            AddVertex16();
            break;
        case 0x24:
            AddVertex10();
            break;
        case 0x25:
            SetVertexXY();
            break;
        case 0x26:
            SetVertexXZ();
            break;
        case 0x27:
            SetVertexYZ();
            break;
        case 0x28:
            SetRelativeVertexCoordinates();
            break;
        case 0x29:
            SetPolygonAttributes();
            break;
        case 0x2A:
            SetTextureParameters();
            break;
        case 0x2B:
            SetTexturePaletteAddress();
            break;
        case 0x30:
            SetDiffuseAmbientReflect();
            break;
        case 0x31:
            SetSpecularReflectEmission();
            break;
        case 0x32:
            SetLightVector();
            break;
        case 0x33:
            SetLightColour();
            break;
        case 0x34:
            SetShininess();
            break;
        case 0x40:
            BeginVertexList();
            break;
        case 0x41:
            EndVertexList();
            break;
        case 0x50:
            SwapBuffers();
            break;
        case 0x60:
            SetViewport();
            break;
        case 0x70:
            BoxTest();
            break;
        default:
            log_fatal("[Geometry Engine] Unknown geometry command %02x", command);
            if (param_table[command] == 0) {
                DequeueEntry();
            } else {
                for (int i = 0; i < param_table[command]; i++) {
                    DequeueEntry();
                }
            }

            break;
        }

        busy = true;
        gpu->system.scheduler.AddEvent(1, &geometry_command_event);
    }
}

void GeometryEngine::CheckGXFIFOInterrupt() {
    switch ((gxstat >> 30) & 0x3) {
    case 1:
        // trigger interrupt if fifo is less than half full
        if (fifo.size() < 128) {
            gpu->system.cpu_core[1].SendInterrupt(InterruptType::GXFIFO);
        }
        break;
    case 2:
        // trigger interrupt if fifo is empty
        if (fifo.size() == 0) {
            gpu->system.cpu_core[1].SendInterrupt(InterruptType::GXFIFO);
        }
        break;
    }
}

void GeometryEngine::UpdateClipMatrix() {
    clip = MultiplyMatrixMatrix(modelview.current, projection.current);
}

Matrix GeometryEngine::MultiplyMatrixMatrix(const Matrix& a, const Matrix& b) {
    Matrix new_matrix;
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            s64 result = 0;
            for (int i = 0; i < 4; i++) {
                result += ((s64)a.field[y][i] * b.field[i][x]);
            }

            new_matrix.field[y][x] = result >> 12;
        }
    }
    return new_matrix;
}

auto GeometryEngine::MultiplyVertexMatrix(const Vertex& a, const Matrix& b) -> Vertex {
    Vertex new_vertex = a;
    new_vertex.x = ((s64)a.x * b.field[0][0] + (s64)a.y * b.field[1][0] + (s64)a.z * b.field[2][0] + (s64)a.w * b.field[3][0]) >> 12;
    new_vertex.y = ((s64)a.x * b.field[0][1] + (s64)a.y * b.field[1][1] + (s64)a.z * b.field[2][1] + (s64)a.w * b.field[3][1]) >> 12;
    new_vertex.z = ((s64)a.x * b.field[0][2] + (s64)a.y * b.field[1][2] + (s64)a.z * b.field[2][2] + (s64)a.w * b.field[3][2]) >> 12;
    new_vertex.w = ((s64)a.x * b.field[0][3] + (s64)a.y * b.field[1][3] + (s64)a.z * b.field[2][3] + (s64)a.w * b.field[3][3]) >> 12;

    return new_vertex;
}

void GeometryEngine::PrintMatrix(const Matrix& a) {
    printf("| %d %d %d %d |\n", a.field[0][0], a.field[0][1], a.field[0][2], a.field[0][3]);
    printf("| %d %d %d %d |\n", a.field[1][0], a.field[1][1], a.field[1][2], a.field[1][3]);
    printf("| %d %d %d %d |\n", a.field[2][0], a.field[2][1], a.field[2][2], a.field[2][3]);
    printf("| %d %d %d %d |\n", a.field[3][0], a.field[3][1], a.field[3][2], a.field[3][3]);
}

void GeometryEngine::PrintVertex(const Vertex& a) {
    printf("| %d |\n", a.x);
    printf("| %d |\n", a.y);
    printf("| %d |\n", a.z);
    printf("| %d |\n", a.w);
}

void GeometryEngine::DoSwapBuffers() {
    for (int i = 0; i < vertex_ram_size; i++) {
        gpu->render_engine.vertex_ram[i] = vertex_ram[i];
    }

    for (int i = 0; i < polygon_ram_size; i++) {
        gpu->render_engine.polygon_ram[i] = polygon_ram[i];
    }

    gpu->render_engine.vertex_ram_size = vertex_ram_size;
    gpu->render_engine.polygon_ram_size = polygon_ram_size;

    vertex_ram_size = 0;
    vertex_count = 0;
    polygon_ram_size = 0;
}

void GeometryEngine::AddVertex() {
    if (vertex_ram_size >= 6144) {
        return;
    }

    current_vertex.w = 1 << 12;
    vertex_ram[vertex_ram_size] = MultiplyVertexMatrix(current_vertex, clip);
    vertex_ram[vertex_ram_size] = NormaliseVertex(vertex_ram[vertex_ram_size]);
    vertex_ram_size++;
    vertex_count++;

    switch (polygon_type) {
    case PolygonType::Triangle:
        if (vertex_count % 3 == 0) {
            AddPolygon();
        }
        break;
    case PolygonType::Quad:
        if (vertex_count % 4 == 0) {
            AddPolygon();
        }
        break;
    case PolygonType::TriangleStrip:
        if (vertex_count >= 3) {
            AddPolygon();
        }
        break;
    case PolygonType::QuadStrip:
        if ((vertex_count >= 4) && (vertex_count % 2 == 0)) {
            AddPolygon();
        }
        break;
    }
}

void GeometryEngine::AddPolygon() {
    if (polygon_ram_size >= 2048) {
        return;
    }

    int size = 3 + (static_cast<int>(polygon_type) & 0x1);
    current_polygon.size = size;
    current_polygon.vertices = &vertex_ram[vertex_ram_size - size];
    current_polygon.texture_parameters = texture_parameters;
    polygon_ram[polygon_ram_size++] = current_polygon;
}

u32 GeometryEngine::ReadClipMatrix(u32 addr) {
    int x = (addr - 0x04000640) % 4;
    int y = (addr - 0x04000640) / 4;

    return clip.field[y][x];
}

u32 GeometryEngine::ReadVectorMatrix(u32 addr) {
    int x = (addr - 0x04000680) % 3;
    int y = (addr - 0x04000680) / 3;

    return direction.current.field[y][x];
}

Vertex GeometryEngine::NormaliseVertex(Vertex vertex) {
    Vertex render_vertex = vertex;

    if (vertex.w != 0) {
        render_vertex.x = (( vertex.x * 128) / vertex.w) + 128;
        render_vertex.y = ((-vertex.y * 96)  / vertex.w) + 96;
    }

    return render_vertex;
}