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
    busy = false;
    matrix_mode = 0;
    modelview_pointer = 0;
    vertex_ram_size = 0;

    std::queue<Entry> empty_fifo_queue;
    fifo.swap(empty_fifo_queue);

    std::queue<Entry> empty_pipe_queue;
    pipe.swap(empty_pipe_queue);

    state = GeometryEngineState::Running;
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
    data |= (busy << 27);

    return data;
}

void GeometryEngine::WriteGXSTAT(u32 data) {
    // TODO: handle side effects later
    gxstat = data;
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

        if (fifo.size() == 256) {
            log_fatal("[Geometry Engine] Handle full fifo");
        }
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

    // don't interpret a command if the fifo and pipe are both empty
    // or we were already interpreting commands
    if ((total_size == 0) || busy) {
        return;
    }

    // don't interpret commands while the geometry engine
    // is halted
    if (state == GeometryEngineState::Halted) {
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
        case 0x15:
            LoadUnitMatrix();
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
        case 0x1C:
            MultiplyTranslation();
            break;
        case 0x29:
            SetPolygonAttributes();
            break;
        case 0x2A:
            SetTextureParameters();
            break;
        case 0x50:
            SwapBuffers();
            break;
        case 0x60:
            SetViewport();
            break;
        default:
            log_fatal("[Geometry Engine] Handle geometry command %02x", command);
            break;
        }

        // keep on interpreting commands
        // now that we interpreted a command bit 27 is 0 
        // and no commands are being interpreted
        busy = false;
        gpu->hw->scheduler.Add(1, InterpretCommandEvent);
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
    clip_current = MultiplyMatrixMatrix(projection_current, modelview_current);
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

void GeometryEngine::DoSwapBuffers() {
    for (int i = 0; i < vertex_ram_size; i++) {
        gpu->render_engine.vertex_ram[i] = vertex_ram[i];
    }

    vertex_ram_size = 0;

    // unhalt the geometry engine
    state = GeometryEngineState::Running;
}