#include "common/logger.h"
#include "common/bits.h"
#include "nds/video/gpu/gpu.h"

namespace nds {

static constexpr std::array<int, 256> parameter_table = {{
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

GPU::GPU(common::Scheduler& scheduler) : scheduler(scheduler) {}

void GPU::reset() {
    framebuffer.fill(0);
    disp3dcnt.data = 0;
    gxstat.data = 0; 
    gxfifo.reset();
    gxpipe.reset();
    busy = false;

    geometry_command_event = scheduler.register_event("GeometryCommand", [this]() {
        busy = false;
        execute_command();
    });

    matrix_mode = MatrixMode::Projection;
}

void GPU::write_disp3dcnt(u32 value, u32 mask) {
    disp3dcnt.data = (disp3dcnt.data & ~mask) | (value & mask);
}

u32 GPU::read_gxstat() {
    u32 value = 0;
    value |= gxfifo.get_size() << 16;

    if (gxfifo.is_full()) {
        value |= 1 << 24;
    }

    if (gxfifo.get_size() < 128) {
        value |= 1 << 25;
    }

    if (gxfifo.is_empty()) {
        value |= 1 << 26;
    }

    value |= gxstat.fifo_irq << 30;
    return value;
}

void GPU::write_gxstat(u32 value, u32 mask) {
    gxstat.data = (gxstat.data & ~mask) | (value & mask);
    check_gxfifo_irq();
}

void GPU::queue_command(u32 addr, u32 data) {
    u8 command = common::get_field<2, 7>(addr);
    queue_entry({command, data});
}

void GPU::render() {
    // TODO: ideally we should render scanline by scanline
    // figure out how this works on real hardware
}

void GPU::check_gxfifo_irq() {
    if (gxstat.fifo_irq == InterruptType::LessThanHalfFull && gxfifo.get_size() < 128) {
        logger.todo("send less than half full gxfifo irq");
    } else if (gxstat.fifo_irq == InterruptType::Empty && gxfifo.is_empty()) {
        logger.todo("send empty gxfifo irq");
    }
}

void GPU::queue_entry(Entry entry) {
    if (gxfifo.is_empty() && !gxpipe.is_full()) {
        gxpipe.push(entry);
    } else {
        gxfifo.push(entry);

        if (gxfifo.is_full()) {
            logger.todo("gxfifo is full");
        }
    }

    execute_command();
}

GPU::Entry GPU::dequeue_entry() {
    auto entry = gxpipe.pop();

    // if the pipe is running half empty
    // then move 2 entries from the fifo to the pipe
    if (gxpipe.get_size() < 3) {
        if (!gxfifo.is_empty()) {
            gxpipe.push(gxfifo.pop());
        }

        if (!gxfifo.is_empty()) {
            gxpipe.push(gxfifo.pop());
        }

        check_gxfifo_irq();

        if (gxfifo.get_size() < 128) {
            logger.warn("trigger gxfifo dma");
        }
    }

    return entry;
}

void GPU::execute_command() {
    auto total_size = gxfifo.get_size() + gxpipe.get_size();
    if (busy || (total_size == 0)) {
        return;
    }

    u8 command = gxpipe.get_front().command;
    u8 parameter_count = parameter_table[command];

    if (total_size >= parameter_count) {
        switch (command) {
        case 0x10:
            set_matrix_mode();
            break;
        default:
            logger.todo("GPU: handle command %02x", command);
        }

        busy = true;
        scheduler.add_event(1, &geometry_command_event);
    }
}

} // namespace nds