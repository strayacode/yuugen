#include "Common/Memory.h"
#include "VideoCommon/Renderer3D.h"
#include "VideoCommon/GPU.h"
#include "Core/system.h"

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

Renderer3D::Renderer3D(GPU& gpu) : gpu(gpu) {}

void Renderer3D::reset() {
    disp3dcnt = 0;
    clear_depth = 0;
    clear_colour = 0;
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

    geometry_command_event = gpu.system.scheduler.RegisterEvent("GeometryCommand", [this]() {
        busy = false;
        run_command();
    });

    texture_attributes.parameters = 0;
    texture_attributes.palette_base = 0;
    clrimage_offset = 0;
    fog_colour = 0;
    fog_offset = 0;
    edge_colour.fill(0);
    fog_table.fill(0);
    toon_table.fill(0);
    alpha_test_ref = 0;
}

u8 Renderer3D::read_byte(u32 addr) {
    switch (addr) {
    default:
        log_fatal("Renderer3D: %08x", addr);
    }

    return 0;
}

u16 Renderer3D::read_half(u32 addr) {
    switch (addr) {
    default:
        log_fatal("Renderer3D: %08x", addr);
    }

    return 0;
}

u32 Renderer3D::read_word(u32 addr) {
    switch (addr) {
    case 0x040004A4:
        // is polygon attr even readable?
        return 0;
    case 0x04000600:
        return read_gxstat();
    }

    if (Common::in_range(0x04000640, 0x04000680, addr)) {
        return read_clip_matrix(addr);
    }

    log_warn("Renderer3D: handle word read %08x", addr);

    return 0;
}


void Renderer3D::write_byte(u32 addr, u8 data) {
    switch (addr) {
    default:
        log_fatal("Renderer3D: %08x", addr);
    }
}

void Renderer3D::write_half(u32 addr, u16 data) {
    switch (addr) {
    case 0x04000340:
        alpha_test_ref = data;
        return;
    case 0x04000354:
        clear_depth = data;
        return;
    case 0x04000356:
        clrimage_offset = data;
        return;
    case 0x0400035C:
        fog_offset = data;
        return;
    }

    if (Common::in_range(0x04000380, 0x040003C0, addr)) {
        Common::write<u16>(fog_table.data(), addr - 0x04000380, data);
        return;
    }

    log_warn("Renderer3D: handle half write %08x = %04x", addr, data);
}

void Renderer3D::write_word(u32 addr, u32 data) {
    switch (addr) {
    case 0x04000350:
        clear_colour = data;
        return;
    case 0x04000358:
        fog_colour = data;
        return;
    case 0x04000600:
        gxstat = data;
        return; 
    }

    if (Common::in_range(0x04000330, 0x04000340, addr)) {
        Common::write<u32>(edge_colour.data(), addr - 0x04000330, data);
        return;
    }

    if (Common::in_range(0x04000360, 0x04000380, addr)) {
        Common::write<u32>(fog_table.data(), addr - 0x04000360, data);
        return;
    }

    if (Common::in_range(0x04000380, 0x040003C0, addr)) {
        Common::write<u32>(fog_table.data(), addr - 0x04000380, data);
        return;
    }

    if (Common::in_range(0x04000400, 0x04000440, addr)) {
        write_gxfifo(data);
        return;
    }

    if (Common::in_range(0x04000440, 0x040005CC, addr)) {
        queue_command(addr, data);
        return;
    }

    log_warn("Renderer3D: handle word write %08x = %08x", addr, data);
}

void Renderer3D::queue_command(u32 addr, u32 data) {
    u8 command = (addr >> 2) & 0x7F;
    
    queue_entry({command, data});
}

void Renderer3D::queue_entry(Entry entry) {
    if (fifo.size() == 0 && pipe.size() < 4) {
        pipe.push(entry);
    } else {
        fifo.push(entry);

        while (fifo.size() >= 256) {
            // just run commands until the fifo isn't full
            busy = false;
            gpu.system.scheduler.CancelEvent(&geometry_command_event);
            run_command();
        }
    }

    run_command();
}

Entry Renderer3D::dequeue_entry() {
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

        check_gxfifo_interrupt();

        // TODO: do dma stuff
    }

    return entry;
}

void Renderer3D::run_command() {
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
            log_fatal("Renderer3D: unknown geometry command %02x", command);
        }

        busy = true;
        gpu.system.scheduler.AddEvent(1, &geometry_command_event);
    }
}

u32 Renderer3D::read_gxstat() {
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

void Renderer3D::check_gxfifo_interrupt() {
    switch ((gxstat >> 30) & 0x3) {
    case 1:
        // trigger interrupt if fifo is less than half full
        if (fifo.size() < 128) {
            gpu.system.cpu_core[1].SendInterrupt(InterruptType::GXFIFO);
        }
        break;
    case 2:
        // trigger interrupt if fifo is empty
        if (fifo.size() == 0) {
            gpu.system.cpu_core[1].SendInterrupt(InterruptType::GXFIFO);
        }
        break;
    }
}

void Renderer3D::add_vertex() {
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
            add_polygon();
        }
        break;
    case PolygonType::Quad:
        if (vertex_count % 4 == 0) {
            add_polygon();
        }
        break;
    case PolygonType::TriangleStrip:
        if (vertex_count >= 3) {
            add_polygon();
        }
        break;
    case PolygonType::QuadStrip:
        if ((vertex_count >= 4) && (vertex_count % 2 == 0)) {
            add_polygon();
        }
        break;
    }
}

void Renderer3D::add_polygon() {
    if (polygon_ram_size >= 2048) {
        return;
    }

    int size = 3 + (static_cast<int>(polygon_type) & 0x1);
    current_polygon.size = size;
    current_polygon.vertices = &vertex_ram[vertex_ram_size - size];
    current_polygon.texture_attributes = texture_attributes;
    polygon_ram[polygon_ram_size++] = current_polygon;
}

Vertex Renderer3D::NormaliseVertex(Vertex vertex) {
    Vertex render_vertex = vertex;

    if (vertex.w != 0) {
        render_vertex.x = (( vertex.x * 128) / vertex.w) + 128;
        render_vertex.y = ((-vertex.y * 96)  / vertex.w) + 96;
    }

    return render_vertex;
}

Matrix Renderer3D::MultiplyMatrixMatrix(const Matrix& a, const Matrix& b) {
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

Vertex Renderer3D::MultiplyVertexMatrix(const Vertex& a, const Matrix& b) {
    Vertex new_vertex = a;
    new_vertex.x = ((s64)a.x * b.field[0][0] + (s64)a.y * b.field[1][0] + (s64)a.z * b.field[2][0] + (s64)a.w * b.field[3][0]) >> 12;
    new_vertex.y = ((s64)a.x * b.field[0][1] + (s64)a.y * b.field[1][1] + (s64)a.z * b.field[2][1] + (s64)a.w * b.field[3][1]) >> 12;
    new_vertex.z = ((s64)a.x * b.field[0][2] + (s64)a.y * b.field[1][2] + (s64)a.z * b.field[2][2] + (s64)a.w * b.field[3][2]) >> 12;
    new_vertex.w = ((s64)a.x * b.field[0][3] + (s64)a.y * b.field[1][3] + (s64)a.z * b.field[2][3] + (s64)a.w * b.field[3][3]) >> 12;

    return new_vertex;
}

void Renderer3D::update_clip_matrix() {
    clip = MultiplyMatrixMatrix(modelview.current, projection.current);
}

void Renderer3D::do_swap_buffers() {
    for (int i = 0; i < vertex_ram_size; i++) {
        renderer_vertex_ram[i] = vertex_ram[i];
    }

    for (int i = 0; i < polygon_ram_size; i++) {
        renderer_polygon_ram[i] = polygon_ram[i];
    }

    renderer_vertex_ram_size = vertex_ram_size;
    renderer_polygon_ram_size = polygon_ram_size;

    vertex_ram_size = 0;
    vertex_count = 0;
    polygon_ram_size = 0;
}

void Renderer3D::write_gxfifo(u32 data) {
    if (gxfifo == 0) {
        gxfifo = data;
    } else {
        u8 command = gxfifo & 0xFF;
        queue_entry({command, data});

        gxfifo_write_count++;

        if (gxfifo_write_count == param_table[command]) {
            gxfifo >>= 8;
            gxfifo_write_count = 0;
        }
    }

    while ((gxfifo != 0) && (param_table[gxfifo & 0xFF] == 0)) {
        u8 command = gxfifo & 0xFF;
        queue_entry({command, 0});
        gxfifo >>= 8;
    }
}

u32 Renderer3D::read_clip_matrix(u32 addr) {
    int x = (addr - 0x04000640) % 4;
    int y = (addr - 0x04000640) / 4;

    return clip.field[y][x];
}