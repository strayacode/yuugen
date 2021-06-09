#include <core/hw/gpu/engine_3d/geometry_engine.h>
#include <core/hw/gpu/gpu.h>
#include <core/core.h>
#include <array>
#include <tuple>

static constexpr std::array<int, 256> parameter_count = {{
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

GeometryEngine::GeometryEngine(GPU* gpu) : gpu(gpu) {

}

void GeometryEngine::Reset() {
    matrix_mode = 0;
    projection_pointer = 0;
    position_pointer = 0;
    polygon_type = 0;
    vertex_colour = 0;
    vertex_count = 0;
    gxfifo = 0;
    gxfifo_write_count = 0;
    state = STATE_RUNNING;
    // create an empty queue and swap it
    std::queue<Entry> empty_queue;
    fifo.swap(empty_queue);

    vertex_ram.clear();
    vertex_ram.reserve(6144);
    polygon_ram.clear();
}

void GeometryEngine::WriteGXFIFO(u32 data) {
    if (gxfifo == 0) {
        // commands should first be read in
        gxfifo = data;
    } else {
        u8 command = gxfifo & 0xFF;
        // there are commands that need to have parameters read in
        QueueEntry({command, data});

        gxfifo_write_count++;

        if (gxfifo_write_count >= parameter_count[gxfifo & 0xFF]) {
            // now we have recieved enough parameters for a particular command, so we can move onto another command
            gxfifo >>= 8;
            gxfifo_write_count = 0;
        }
    }
}

void GeometryEngine::QueueCommand(u32 addr, u32 data) {
    u8 command = (addr >> 2) & 0xFF;

    QueueEntry({command, data});
}

void GeometryEngine::QueueEntry(Entry entry) {
    if (fifo.size() == 0 && pipe.size() < 4) {
        // if fifo is empty and pipe is not full, push to the pipe
        pipe.push(entry);
    } else {
        // before writing to the pipe, if the fifo is full keep executing commands
        if (fifo.size() >= 256) {
            GXSTAT &= ~(1 << 27);
            InterpretCommand();
        }

        if (fifo.size() >= 128 && (GXSTAT & (1 << 25))) {
            GXSTAT &= ~(1 << 25);
        }

        fifo.push(entry);
    }

    InterpretCommand();
}

auto GeometryEngine::DequeueEntry() -> Entry {
    Entry entry = pipe.front();

    pipe.pop();

    // if the pipe is running half empty
    // move 2 entries from the fifo to the pipe
    if (pipe.size() < 3) {
        if (fifo.size() > 0) {
            pipe.push(fifo.front());
            fifo.pop();
        }

        CheckGXFIFOInterrupt();

        if (fifo.size() > 0) {
            pipe.push(fifo.front());
            fifo.pop();
        }

        CheckGXFIFOInterrupt();
    }

    return entry;
}

void GeometryEngine::InterpretCommand() {
    int total_size = fifo.size() + pipe.size();

    if (total_size == 0) {
        return;
    }

    Entry entry = pipe.front();

    u8 param_count = parameter_count[entry.command];
    if (total_size >= param_count) {
        // for now just make sure to remove the entries from the pipe
        DequeueEntry();
        for (int i = 1; i < param_count; i++) {
            DequeueEntry();
        }

        gpu->core->scheduler.Add(2, InterpretCommandTask);
    }

    if (fifo.size() < 128 && !(GXSTAT & (1 << 25))) {
        GXSTAT |= (1 << 25);
    }
}

auto GeometryEngine::MultiplyMatrixMatrix(const Matrix& a, const Matrix& b) -> Matrix {
    Matrix new_matrix;
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            s64 value = 0;
            for (int i = 0; i < 4; i++) {
                value += (s64)a.field[y][i] * (s64)b.field[i][x];
                new_matrix.field[y][x] = (s32)(value >> 12);
            } 
        }
    }
    return new_matrix;
}

auto GeometryEngine::MultiplyVertexMatrix(const Vertex& a, const Matrix& b) -> Vertex {
    Vertex new_vertex;
    new_vertex.x = (s32)((s64)a.x * b.field[0][0] + (s64)a.y * b.field[1][0] + (s64)a.z * b.field[2][0] + (s64)a.w * b.field[3][0]) >> 12;
    new_vertex.y = (s32)((s64)a.x * b.field[0][1] + (s64)a.y * b.field[1][1] + (s64)a.z * b.field[2][1] + (s64)a.w * b.field[3][1]) >> 12;
    new_vertex.z = (s32)((s64)a.x * b.field[0][2] + (s64)a.y * b.field[1][2] + (s64)a.z * b.field[2][2] + (s64)a.w * b.field[3][2]) >> 12;
    new_vertex.w = (s32)((s64)a.x * b.field[0][3] + (s64)a.y * b.field[1][3] + (s64)a.z * b.field[2][3] + (s64)a.w * b.field[3][3]) >> 12;

    return new_vertex;
}

auto GeometryEngine::MultiplyVertexVertex(const Vertex& a, const Vertex& b) -> u32 {
    // pretty much just a dot product lol
    u32 result = (s32)((s64)a.x * b.x + (s64)a.y * b.y + (s64)a.z * b.z + (s64)a.w * b.w) >> 12;

    return result;
}

void GeometryEngine::AddVertex() {
    if (vertex_ram.size() >= 6144) {
        return;
    }

    // first save the vertex to vertex ram
    Vertex result_vertex = MultiplyVertexMatrix(recent_vertex, clip_current);
    vertex_ram.push_back(result_vertex);

    // check whether we should add a polygon
    // switch (polygon_type) {
    // case 0:
    //     // triangle (3 vertices)
    //     if (vertex_ram.size() % 3 == 0) {
    //         AddPolygon();
    //     }
    //     break;
    // case 1:
    //     // quad (4 vertices)
    //     if (vertex_ram.size() % 4 == 0) {
    //         AddPolygon();
    //     }
    //     break;
    // default:
    //     log_fatal("handle polygon type %d", polygon_type);
    // }
}

void GeometryEngine::AddPolygon() {

}

void GeometryEngine::PrintMatrix(const Matrix& a) {
    printf("| %d %d %d %d |\n", a.field[0][0], a.field[0][1], a.field[0][2], a.field[0][3]);
    printf("| %d %d %d %d |\n", a.field[1][0], a.field[1][1], a.field[1][2], a.field[1][3]);
    printf("| %d %d %d %d |\n", a.field[2][0], a.field[2][1], a.field[2][2], a.field[2][3]);
    printf("| %d %d %d %d |\n", a.field[3][0], a.field[3][1], a.field[3][2], a.field[3][3]);
}

void GeometryEngine::UpdateClipMatrix() {
    clip_current = MultiplyMatrixMatrix(position_current, projection_current);
}

void GeometryEngine::DoSwapBuffers() {
    // // clear the render engines vertex and polygon ram before writing to it
    // gpu->render_engine.polygon_ram.clear();
    // gpu->render_engine.vertex_ram.clear();

    // // replace the render engines vertex and polygon ram with the geometry engines and empty the geometry engines
    // // for now we shall just render vertices
    // for (unsigned int i = 0; i < vertex_ram.size(); i++) {
    //     if (vertex_ram[i].w != 0) {
    //         u16 screen_width = (gpu->render_engine.screen_x2 - gpu->render_engine.screen_x1 + 1) & 0x1FF;
    //         u16 screen_height = (gpu->render_engine.screen_y2 - gpu->render_engine.screen_y1 + 1) & 0xFF;
    //         u16 render_x = ((s64)vertex_ram[i].x + vertex_ram[i].w) * screen_width / (2 * vertex_ram[i].w) + gpu->render_engine.screen_x1;
    //         u16 render_y = (-(s64)vertex_ram[i].y + vertex_ram[i].w) * screen_height / (2 * vertex_ram[i].w) + gpu->render_engine.screen_y1;
    //         render_x &= 0x1FF;
    //         render_y &= 0xFF;

    //         Vertex render_vertex;

    //         render_vertex.x = render_x;
    //         render_vertex.y = render_y;
    //         gpu->render_engine.vertex_ram.push_back(render_vertex);
    //         // TODO: update z coord here
    //     }
    // }

    // // empty polygon and vertex ram
    // polygon_ram.clear();
    // vertex_ram.clear();

    // // unhalt the geometry engine
    // state = STATE_RUNNING;
    
    // // schedule more interpret commands events with 1 cycle delay
    // gpu->core->scheduler.Add(1, InterpretCommandTask);
}

void GeometryEngine::CheckGXFIFOInterrupt() {
    switch ((GXSTAT >> 30) & 0x3) {
    case 1:
        // trigger interrupt if fifo is less than half full
        if (fifo.size() < 128) {
            gpu->core->arm9.SendInterrupt(21);
        }
        break;
    case 2:
        // trigger interrupt if fifo is empty
        if (fifo.size() == 0) {
            gpu->core->arm9.SendInterrupt(21);
        }
        break;
    }
}