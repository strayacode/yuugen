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
    switch (addr) {
    case 0x04000440:
        QueueEntry({0x10, data});
        break;
    case 0x04000444:
        QueueEntry({0x11, data});
        break;
    case 0x04000448:
        QueueEntry({0x12, data});
        break;
    case 0x04000454:
        QueueEntry({0x15, data});
        break;
    case 0x04000460:
        QueueEntry({0x18, data});
        break;
    case 0x04000464:
        QueueEntry({0x19, data});
        break;
    case 0x04000468:
        QueueEntry({0x1A, data});
        break;
    case 0x04000470:
        QueueEntry({0x1C, data});
        break;
    case 0x04000480:
        QueueEntry({0x20, data});
        break;
    case 0x0400048C:
        QueueEntry({0x23, data});
        break;
    case 0x040004A4:
        QueueEntry({0x29, data});
        break;
    case 0x040004A8:
        QueueEntry({0x2A, data});
        break;
    case 0x040004AC:
        QueueEntry({0x2B, data});
        break;
    case 0x040004D0:
        QueueEntry({0x34, data});
        break;
    case 0x04000500:
        QueueEntry({0x40, data});
        break;
    case 0x04000504:
        QueueEntry({0x41, data});
        break;
    case 0x04000540:
        QueueEntry({0x50, data});
        break;
    case 0x04000580:
        QueueEntry({0x60, data});
        break;
    default:
        log_fatal("[GeometryEngine] Undefined geometry command defined by address %08x with data %08x", addr, data);
    }
}

void GeometryEngine::QueueEntry(Entry entry) {
    if (fifo.size() == 0 && pipe.size() < 4) {
        // add the entry to the pipe
        pipe.push(entry);
        
    } else {
        // add the entry to the fifo
        fifo.push(entry);

        // update bits 16..24 (for number of fifo entries)
        GXSTAT = (GXSTAT & ~0x1FF0000) | (fifo.size() << 16);

        // fifo is not empty anymore
        GXSTAT &= ~(1 << 26);

        // check if fifo is over half capacity
        if (fifo.size() >= 128) {
            GXSTAT |= (1 << 25);
        }

        // keep executing commands while the fifo is full
        // however only do it if the geometry engine is not halted
        if (state == STATE_RUNNING) {
            while (fifo.size() > 256) {
                InterpretCommand();
            }
        }
    }

    if (state == STATE_RUNNING) {
        InterpretCommand();
    }   
}

auto GeometryEngine::DequeueEntry() -> Entry {
    Entry front_entry = pipe.front();
    pipe.pop();

    // if the pipe is half empty (less than 3 entries)
    // then move the first 2 entries from the fifo to the pipe if any exist
    if (pipe.size() < 3) {
        if (fifo.size()) {
            pipe.push(fifo.front());
            fifo.pop();
        }

        if (fifo.size()) {
            pipe.push(fifo.front());
            fifo.pop();
        }

        // check gxfifo irq
        switch (GXSTAT >> 30) {
        case 0:
            // irq is never sent
            break;
        case 1:
            // less than half full
            if (fifo.size() < 128) {
                gpu->core->arm9.SendInterrupt(21);
            }
            break;
        case 2:
            // fifo empty
            if (!fifo.size()) {
                gpu->core->arm9.SendInterrupt(21);
            }
            break;
        case 3:
            // reserved
            break;
        }
    }

    return front_entry;
}

void GeometryEngine::InterpretCommand() {
    // only interpret a command if there exists enough entries in both fifo and pipe
    // for a command to be executed successfully
    int total_size = fifo.size() + pipe.size();

    // don't execute any commands if none are in fifo or pipe
    if (total_size == 0) {
        return;
    }

    Entry entry = pipe.front();

    u8 param_count = parameter_count[entry.command];
    if (total_size >= param_count) {
        switch (entry.command) {
        case 0x00:
            // used to pad packed commands in gxfifo
            DequeueEntry();
            break;
        case 0x10:
            CommandSetMatrixMode();
            break;
        case 0x11:
            CommandPushCurrentMatrix();
            break;
        case 0x12:
            CommandPopCurrentMatrix();
            break;
        case 0x13:
            CommandStoreCurrentMatrix();
            break;
        case 0x14:
            CommandRestoreCurrentMatrix();
            break;
        case 0x15:
            CommandLoadUnitMatrix();
            break;
        case 0x16:
            CommandLoad4x4();
            break;
        case 0x17:
            CommandLoad4x3();
            break;
        case 0x18:
            CommandMultiply4x4();
            break;
        case 0x19:
            CommandMultiply4x3();
            break;
        case 0x1A:
            CommandMultiply3x3();
            break;
        case 0x1B:
            CommandMultiplyScale();
            break;
        case 0x1C:
            CommandMultiplyTranslation();
            break;
        case 0x20:
            CommandSetVertexColour();
            break;
        case 0x22:
            CommandSetTextureCoordinates();
            break;
        case 0x23:
            CommandAddVertex16();
            break;
        case 0x26:
            CommandSetVertexXZ();
            break;
        case 0x28:
            CommandSetVertexRelative();
            break;
        case 0x29:
            CommandSetPolygonAttributes();
            break;
        case 0x2A:
            CommandSetTextureParameters();
            break;
        case 0x2B:
            CommandSetTexturePaletteAddress();
            break;
        case 0x30:
            CommandSetDiffuseAmbientReflect();
            break;
        case 0x31:
            CommandSetSpecularReflectEmission();
            break;
        case 0x32:
            CommandSetLightDirectionVector();
            break;
        case 0x33:
            CommandSetLightColour();
            break;
        case 0x34:
            CommandShininess();
            break;
        case 0x40:
            CommandBeginVertexList();
            break;
        case 0x41:
            // end vertex list doesn't do anything
            DequeueEntry();
            break;
        case 0x50:
            CommandSwapBuffers();
            break;
        case 0x60:
            CommandSetViewport();
            break;
        default:
            // if (parameter_count[entry.command] == 0) {
            //     DequeueEntry();
            // } else {
            //     for (int i = 0; i < parameter_count[entry.command]; i++) {
            //         DequeueEntry();
            //     }
            // }
            log_fatal("[GeometryEngine] Handle geometry command %02x", entry.command);
            break;
        }
        if (state == STATE_RUNNING) {
            // schedule more interpret commands events with 1 cycle delay
            gpu->core->scheduler.Add(1, InterpretCommandTask);
        }
    }

    if (fifo.size() < 128 && !(GXSTAT & (1 << 25))) {
        GXSTAT |= (1 << 25);
        gpu->core->dma[1].Trigger(7);
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
    // clear the render engines vertex and polygon ram before writing to it
    gpu->render_engine.polygon_ram.clear();
    gpu->render_engine.vertex_ram.clear();

    // replace the render engines vertex and polygon ram with the geometry engines and empty the geometry engines
    // for now we shall just render vertices
    printf("size %d\n", vertex_ram.size());
    for (unsigned int i = 0; i < vertex_ram.size(); i++) {
        printf("ok\n");
        if (vertex_ram[i].w != 0) {
            u16 screen_width = (gpu->render_engine.screen_x2 - gpu->render_engine.screen_x1 + 1) & 0x1FF;
            u16 screen_height = (gpu->render_engine.screen_y2 - gpu->render_engine.screen_y1 + 1) & 0xFF;
            u16 render_x = ((s64)vertex_ram[i].x + vertex_ram[i].w) * screen_width / (2 * vertex_ram[i].w) + gpu->render_engine.screen_x1;
            u16 render_y = (-(s64)vertex_ram[i].y + vertex_ram[i].w) * screen_height / (2 * vertex_ram[i].w) + gpu->render_engine.screen_y1;
            render_x &= 0x1FF;
            render_y &= 0xFF;

            Vertex render_vertex;

            render_vertex.x = render_x;
            render_vertex.y = render_y;
            gpu->render_engine.vertex_ram.push_back(render_vertex);
            // TODO: update z coord here
        }
    }

    // empty polygon and vertex ram
    polygon_ram.clear();
    vertex_ram.clear();

    // unhalt the geometry engine
    state = STATE_RUNNING;
    
    // schedule more interpret commands events with 1 cycle delay
    gpu->core->scheduler.Add(1, InterpretCommandTask);
}