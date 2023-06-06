#include "common/logger.h"
#include "core/hardware/timers.h"

namespace core {

Timers::Timers(Scheduler& scheduler, IRQ& irq) : scheduler(scheduler), irq(irq) {}

void Timers::reset() {
    channels.fill(Channel{});

    for (int i = 0; i < 4; i++) {
        overflow_events[i] = scheduler.register_event("Timer Overflow", [this, i]() {
            overflow(i);
        });
    }
}

void Timers::write_length(int index, u16 value, u32 mask) {
    channels[index].reload_value = (channels[index].reload_value & ~mask) | (value & mask);
}

void Timers::write_control(int index, u16 value, u32 mask) {
    auto& channel = channels[index];
    if (channel.active) {
        deactivate_channel(index);
    }

    mask &= 0xc7;
    
    auto old_control = channel.control;
    channel.control.data = (channel.control.data & ~mask) | (value & mask);
    channel.shift = shifts[channel.control.prescaler];

    if (channel.control.start) {
        if (!old_control.start) {
            channel.counter = channel.reload_value;
        }

        if (index == 0 || !channel.control.count_up) {
            activate_channel(index);
        }
    }
}

void Timers::overflow(int index) {
    logger.error("Timers: handle overflow");
}

void Timers::activate_channel(int index) {
    auto& channel = channels[index];
    channel.active = true;
    channel.activation_timestamp = scheduler.get_current_time();

    u64 delay = (0x10000 - channel.counter) << channel.shift;
    scheduler.add_event(delay, &overflow_events[index]);
}

void Timers::deactivate_channel(int index) {
    auto& channel = channels[index];
    channel.counter = update_counter(index);
    channel.active = false;

    if (channel.counter >= 0x10000) {
        logger.error("Timers: handle counter greater than 16-bits");
    }

    scheduler.cancel_event(&overflow_events[index]);
}

u16 Timers::update_counter(int index) {
    auto& channel = channels[index];
    if (!channel.active) {
        return channel.counter;
    }

    auto delta = (scheduler.get_current_time() - channel.activation_timestamp) >> channel.shift;
    return channel.counter + delta;
}

} // namespace core