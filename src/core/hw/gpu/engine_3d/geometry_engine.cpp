#include <core/hw/gpu/engine_3d/geometry_engine.h>
#include <core/hw/gpu/gpu.h>
#include <core/hw/hw.h>

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

GeometryEngine::GeometryEngine(GPU* gpu) : gpu(gpu) {
    InterpretCommandEvent = std::bind(&GeometryEngine::InterpretCommand, this);
}

void GeometryEngine::Reset() {
    gxstat = 0;
    gxfifo = 0;
    gxfifo_write_count = 0;
    matrix_mode = 0;
    modelview_pointer = 0;
    vertex_ram_size = 0;
    screen_x1 = screen_x2 = screen_y1 = screen_y2 = 0;

    std::queue<Entry> empty_fifo_queue;
    fifo.swap(empty_fifo_queue);

    std::queue<Entry> empty_pipe_queue;
    pipe.swap(empty_pipe_queue);

    state = GeometryEngineState::Running;

    busy = false;
}

auto GeometryEngine::ReadGXSTAT() -> u32 {
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
    // if (fifo.size() == 0 && pipe.size() < 4) {
    //     pipe.push(entry);
    // } else {
    //     fifo.push(entry);

    //     if (fifo.size() == 256) {
    //         log_fatal("[GeometryEngine] Handle full fifo");
    //     }
    // }

    fifo.push(entry);

    InterpretCommand();
}

auto GeometryEngine::DequeueEntry() -> Entry {
    Entry entry = fifo.front();

    fifo.pop();
    

    // // if the pipe is running half empty
    // // move 2 entries from the fifo to the pipe
    // if (pipe.size() < 3) {
    //     if (fifo.size() > 0) {
    //         pipe.push(fifo.front());
    //         fifo.pop();
    //     }

    //     if (fifo.size() > 0) {
    //         pipe.push(fifo.front());
    //         fifo.pop();
    //     }

    //     CheckGXFIFOInterrupt();

    //     // TODO: do dma stuff
    // }

    return entry;
}

void GeometryEngine::InterpretCommand() {
    // int total_size = fifo.size() + pipe.size();

    // // don't interpret a command if the fifo and pipe are both empty
    // // or we were already interpreting commands
    // if ((total_size == 0) || busy) {
    //     return;
    // }

    // u8 command = pipe.front().command;
    // u8 param_count = param_table[command];

    // if (total_size >= param_count) {
    //     switch (command) {
    //     // case 0x10:
    //     //     SetMatrixMode();
    //     //     break;
    //     // case 0x11:
    //     //     PushCurrentMatrix();
    //     //     break;
    //     // case 0x12:
    //     //     PopCurrentMatrix();
    //     //     break;
    //     // case 0x15:
    //     //     LoadUnitMatrix();
    //     //     break;
    //     // case 0x18:
    //     //     Multiply4x4();
    //     //     break;
    //     // case 0x19:
    //     //     Multiply4x3();
    //     //     break;
    //     // case 0x1A:
    //     //     Multiply3x3();
    //     //     break;
    //     // case 0x1C:
    //     //     MultiplyTranslation();
    //     //     break;
    //     // case 0x20:
    //     //     SetVertexColour();
    //     //     break;
    //     // case 0x23:
    //     //     AddVertex16();
    //     //     break;
    //     // case 0x29:
    //     //     SetPolygonAttributes();
    //     //     break;
    //     // case 0x2A:
    //     //     SetTextureParameters();
    //     //     break;
    //     // case 0x40:
    //     //     BeginVertexList();
    //     //     break;
    //     // case 0x41:
    //     //     EndVertexList();
    //     //     break;
    //     // case 0x50:
    //     //     SwapBuffers();
    //     //     break;
    //     // case 0x60:
    //     //     SetViewport();
    //     //     break;
    //     default:
    //         // log_fatal("[GeometryEngine] Handle geometry command %02x", command);
    //         if (param_table[command] == 0) {
    //             DequeueEntry();
    //         } else {
    //             for (int i = 0; i < param_table[command]; i++) {
    //                 DequeueEntry();
    //             }
    //         }
    //         break;
    //     }

    //     // keep on interpreting commands
    //     // now that we interpreted a command bit 27 is 0 
    //     // and no commands are being interpreted
    //     busy = true;
    //     gpu->hw->scheduler.Add(1, [this]() {
    //         busy = false;
    //         InterpretCommand();
    //     });
    // }

    if (busy) {
        return;
    }

    if (fifo.size() > 0) {
        DequeueEntry();

        CheckGXFIFOInterrupt();

        busy = true;
        gpu->hw->scheduler.Add(1, [this]() {
            busy = false;
            InterpretCommand();
        });
    }   
}

void GeometryEngine::CheckGXFIFOInterrupt() {
    switch ((gxstat >> 30) & 0x3) {
    case 1:
        // trigger interrupt if fifo is less than half full
        if (fifo.size() < 128) {
            gpu->hw->cpu_core[1]->SendInterrupt(21);
        }
        break;
    case 2:
        // trigger interrupt if fifo is empty
        if (fifo.size() == 0) {
            gpu->hw->cpu_core[1]->SendInterrupt(21);
        }
        break;
    }
}

void GeometryEngine::UpdateClipMatrix() {
    clip_current = MultiplyMatrixMatrix(modelview_current, projection_current);
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
        Vertex render_vertex;

        if (vertex_ram[i].w != 0) {
            u16 screen_width = (screen_x2 - screen_x1 + 1) & 0x1FF;
            u16 screen_height = (screen_y2 - screen_y1 + 1) & 0xFF;
            s64 render_x = (((s64)(vertex_ram[i].x + vertex_ram[i].w) * screen_width) / (2 * vertex_ram[i].w)) + screen_x1;
            s64 render_y = (((-(s64)vertex_ram[i].y + vertex_ram[i].w) * screen_height) / (2 * vertex_ram[i].w)) + screen_y1;
            // TODO: update z coord here
            render_x &= 0x1FF;
            render_y &= 0xFF;

            render_vertex.x = render_x;
            render_vertex.y = render_y;
        } else {
            render_vertex.x = 0;
            render_vertex.y = 0;
            render_vertex.z = 0;
        }

        gpu->render_engine.vertex_ram[i] = render_vertex;
    }

    gpu->render_engine.vertex_ram_size = vertex_ram_size;

    vertex_ram_size = 0;
}

void GeometryEngine::AddVertex() {
    if (vertex_ram_size >= 6144) {
        return;
    }

    current_vertex.w = 1 << 12;
    vertex_ram[vertex_ram_size] = current_vertex;
    vertex_ram[vertex_ram_size] = MultiplyVertexMatrix(vertex_ram[vertex_ram_size], clip_current);

    vertex_ram_size++;
}

u32 GeometryEngine::ReadClipMatrix(u32 addr) {
    int x = (addr - 0x04000640) % 4;
    int y = (addr - 0x04000640) / 4;

    return clip_current.field[y][x];
}

u32 GeometryEngine::ReadVectorMatrix(u32 addr) {
    int x = (addr - 0x04000680) % 3;
    int y = (addr - 0x04000680) / 3;

    return direction_current.field[y][x];
}