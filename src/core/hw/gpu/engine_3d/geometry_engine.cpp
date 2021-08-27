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

    std::queue<Entry> empty_fifo_queue;
    fifo.swap(empty_fifo_queue);

    std::queue<Entry> empty_pipe_queue;
    pipe.swap(empty_pipe_queue);
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
            log_fatal("[GPU3D] Handle full fifo");
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

    u8 command = pipe.front().command;
    u8 param_count = param_table[command];

    if (total_size >= param_count) {
        switch (command) {
        case 0x10:
            SetMatrixMode();
            break;
        default:
            log_fatal("[GPU3D] Handle geometry command %02x", command);
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