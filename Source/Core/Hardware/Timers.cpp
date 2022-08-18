#include "Common/format.h"
#include "Common/Log.h"
#include "Core/Hardware/Timers.h"
#include "Core/Core.h"

Timers::Timers(System& system, int arch) : system(system), arch(arch) {}

void Timers::reset() {
    for (int i = 0; i < 4; i++) {
        timer[i].control.data = 0;
        timer[i].counter = 0;
        timer[i].reload_value = 0;
        timer[i].activation_time = 0;
        timer[i].active = false;
        timer[i].shift = 0;

        overflow_event[i] = system.scheduler.RegisterEvent(format("[ARM%d] Timer Overflow %d", arch == 1 ? 9 : 7, i), [this, i]() {
            overflow(i);
        });
    }
}

void Timers::build_mmio(MMIO& mmio) {
    int channel_size = 4;

    for (int i = 0; i < 4; i++) {
        mmio.register_mmio<u16>(
            0x04000100 + (channel_size * i),
            mmio.complex_read<u16>([this, i](u32) {
                return read_counter(i);
            }),
            mmio.complex_write<u16>([this, i](u32, u16 data) {
                write_counter(i, data);
            })
        );

        mmio.register_mmio<u16>(
            0x04000102 + (channel_size * i),
            mmio.complex_read<u16>([this, i](u32) {
                return read_control(i);
            }),
            mmio.complex_write<u16>([this, i](u32, u16 data) {
                write_control(i, data);
            })
        );
    }
}

void Timers::write_counter(int channel, u16 data) {
    timer[channel].reload_value = data;
}

void Timers::write_control(int channel, u16 data) {
    // since channel is being reconfigured cancel the previous overflow event
    if (timer[channel].active) {
        deactivate_channel(channel);
    }
    
    bool old_start = timer[channel].control.start;
    timer[channel].control.data = (timer[channel].control.data & ~0xC7) | (data & 0xC7);
    timer[channel].shift = shifts[timer[channel].control.prescaler];

    if (timer[channel].control.start) {
        // reload the timer counter if going from stopped to started
        if (!old_start) {
            timer[channel].counter = timer[channel].reload_value;
        }

        // reactivate the timer if count up mode disabled
        if (channel == 0 || !timer[channel].control.count_up) {
            activate_channel(channel);
        }
    }
}

void Timers::overflow(int channel) {
    timer[channel].counter = timer[channel].reload_value;

    if (timer[channel].control.irq) {
        system.cpu_core[arch].SendInterrupt(static_cast<InterruptType>(static_cast<int>(InterruptType::Timer0) + channel));
    }

    // reactivate the timer if count up mode disabled
    if (channel == 0 || !timer[channel].control.count_up) {
        activate_channel(channel);
    }
    
    if (channel < 3) {
        if (timer[channel + 1].control.count_up && timer[channel + 1].control.start) {
            // in count up mode the next timer is incremented on overflow
            if (++timer[channel + 1].counter == 0x10000) {
                overflow(channel + 1);
            }
        }
    }
}

void Timers::activate_channel(int channel) {
    timer[channel].active = true;
    timer[channel].activation_time = system.scheduler.GetCurrentTime();

    // determine when channel will overflow and place on scheduler
    u64 delay = (0x10000 - timer[channel].counter) << timer[channel].shift;

    system.scheduler.AddEvent(delay, &overflow_event[channel]);
}

void Timers::deactivate_channel(int channel) {
    timer[channel].counter = update_counter(channel);
    timer[channel].active = false;

    if (timer[channel].counter >= 0x10000) {
        log_fatal("handle");
    }

    system.scheduler.CancelEvent(&overflow_event[channel]);
}

u16 Timers::read_counter(int channel) {
    return update_counter(channel);
}

u16 Timers::read_control(int channel) {
    return timer[channel].control.data;
}

u16 Timers::update_counter(int channel) {
    u16 counter = timer[channel].counter;

    if (!timer[channel].active) {
        return counter;
    }

    u16 change = (system.scheduler.GetCurrentTime() - timer[channel].activation_time) >> timer[channel].shift;
    
    return counter + change;
}