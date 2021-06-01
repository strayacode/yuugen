#include <core/hw/gpu/engine_3d/geometry_engine.h>
#include <core/hw/gpu/gpu.h>
#include <core/core.h>
#include <array>
#include <tuple>

static constexpr std::array<std::pair<int, int>, 38> parameter_count {{
    {0x00, 0},
    {0x10, 1},
    {0x11, 0},
    {0x12, 1},
    {0x13, 1},
    {0x14, 1},
    {0x15, 0},
    {0x16, 16},
    {0x17, 12},
    {0x18, 16},
    {0x19, 12},
    {0x1A, 9},
    {0x1B, 3},
    {0x1C, 3},
    {0x20, 1},
    {0x21, 1},
    {0x22, 1},
    {0x23, 2},
    {0x24, 1},
    {0x25, 1},
    {0x26, 1},
    {0x27, 1},
    {0x28, 1},
    {0x29, 1},
    {0x2A, 1},
    {0x2B, 1},
    {0x30, 1},
    {0x31, 1},
    {0x32, 1},
    {0x33, 1},
    {0x34, 32},
    {0x40, 1},
    {0x41, 0},
    {0x50, 1},
    {0x60, 1},
    {0x70, 3},
    {0x71, 2},
    {0x72, 1},
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
}

void GeometryEngine::WriteGXFIFO(u32 data) {
    if (gxfifo == 0) {
        // commands should first be read in
        gxfifo = data;
    } else {
        // there are commands that need to have parameters read in
        QueueEntry({gxfifo & 0xFF, data});

        gxfifo_write_count++;

        if (gxfifo_write_count >= parameter_count[gxfifo & 0xFF].second) {
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

    u8 param_count = parameter_count[entry.command].second;

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
        case 0x15:
            CommandLoadUnitMatrix();
            break;
        case 0x16:
            CommandLoad4x4();
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
        case 0x1C:
            CommandMultiplyTranslation();
            break;
        case 0x20:
            CommandSetVertexColour();
            break;
        case 0x23:
            CommandAddVertex16();
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
            log_fatal("[GeometryEngine] Handle geometry command %02x", entry.command);
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

auto GeometryEngine::MatrixMultiply(const Matrix& a, const Matrix& b) -> Matrix {
    Matrix new_matrix;
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            // each cell can be calculated with 4 multiplications
            new_matrix.field[y][x] = a.field[y][0] * b.field[0][x] + a.field[y][1] * b.field[1][x] + a.field[y][2] * b.field[2][x] + a.field[y][3] * b.field[3][x];
        }
    }

    return new_matrix;
}

void GeometryEngine::AddVertex(Vertex v) {
    if (vertex_count >= 6144) {
        return;
    }

    printf("add %d %d %d\n", v.x, v.y, v.z);

    // save the vertex to vertex ram
    vertex_ram[vertex_count] = v;

    vertex_count++;
}

void GeometryEngine::UpdateClipMatrix() {
    clip_current = MatrixMultiply(position_current, projection_current);
}