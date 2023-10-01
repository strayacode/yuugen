#include "common/logger.h"
#include "nds/hardware/timers.h"

namespace nds {

Timers::Timers(Scheduler& scheduler, IRQ& irq) : scheduler(scheduler), irq(irq) {}

void Timers::reset() {
    channels.fill(Channel{});

    for (int i = 0; i < 4; i++) {
        overflow_events[i] = scheduler.register_event("Timer Overflow", [this, i]() {
            overflow(i);
        });
    }
}

void Timers::write_length(int id, u16 value, u32 mask) {
    channels[id].reload_value = (channels[id].reload_value & ~mask) | (value & mask);
}

void Timers::write_control(int id, u16 value, u32 mask) {
    auto& channel = channels[id];
    if (channel.active) {
        deactivate_channel(id);
    }

    mask &= 0xc7;
    
    auto old_control = channel.control;
    channel.control.data = (channel.control.data & ~mask) | (value & mask);
    channel.shift = shifts[channel.control.prescaler];

    if (channel.control.start) {
        if (!old_control.start) {
            channel.counter = channel.reload_value;
        }

        if (id == 0 || !channel.control.count_up) {
            activate_channel(id);
        }
    }
}

void Timers::overflow(int id) {
    auto& channel = channels[id];
    channel.counter = channel.reload_value;

    if (channel.control.irq) {
        irq.raise(static_cast<IRQ::Source>(static_cast<int>(IRQ::Source::Timer0) + id));
    }

    if (id == 0 || !channel.control.count_up) {
        activate_channel(id);
    }

    if (id < 3) {
        if (channels[id + 1].control.count_up && channels[id + 1].control.start) {
            // in count up mode the next timer is incremented on overflow
            if (++channels[id + 1].counter == 0x10000) {
                overflow(id + 1);
            }
        }
    }
}

void Timers::activate_channel(int id) {
    auto& channel = channels[id];
    channel.active = true;
    channel.activation_timestamp = scheduler.get_current_time();

    u64 delay = (0x10000 - channel.counter) << channel.shift;
    scheduler.add_event(delay, &overflow_events[id]);
}

void Timers::deactivate_channel(int id) {
    auto& channel = channels[id];
    channel.counter = update_counter(id);
    channel.active = false;

    if (channel.counter >= 0x10000) {
        logger.error("Timers: handle counter greater than 16-bits");
    }

    scheduler.cancel_event(&overflow_events[id]);
}

u16 Timers::update_counter(int id) {
    auto& channel = channels[id];
    if (!channel.active) {
        return channel.counter;
    }

    auto delta = (scheduler.get_current_time() - channel.activation_timestamp) >> channel.shift;
    return channel.counter + delta;
}

} // namespace nds