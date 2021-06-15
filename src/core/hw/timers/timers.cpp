#include <core/hw/timers/timers.h>
#include <core/core.h>

Timers::Timers(Core* core, int arch) : core(core), arch(arch) {

}

void Timers::Reset() {
    for (int i = 0; i < 4; i++) {
        memset(&timer[i], 0, sizeof(Timer));
    }

    enabled = 0;
}

void Timers::WriteTMCNT_L(int timer_index, u16 data) {
    timer[timer_index].reload_value = data;
}

void Timers::WriteTMCNT_H(int timer_index, u16 data) {
    // if count up timing is enabled the prescalar value is ignored so instead the timer increments when the previous counter overflows? hmm
    // count up timing cant be used on timer 0 too
    
    // set bits 0..1 to the number of cycles that must pass before the counter can increment by 1
    // in a specific timer channel
    switch (data & 0x3) {
    case 0:
        timer[timer_index].cycles_per_count = 1;
        timer[timer_index].cycles_left = 1;
        break;
    case 1:
        timer[timer_index].cycles_per_count = 64;
        timer[timer_index].cycles_left = 64;
        break;
    case 2:
        timer[timer_index].cycles_per_count = 256;
        timer[timer_index].cycles_left = 256;
        break;
    case 3:
        timer[timer_index].cycles_per_count = 1024;
        timer[timer_index].cycles_left = 1024;
        break;
    }


    // if the timer has gone from disabled to enabled then reload tmcnt_l with the reload value
    if (!(timer[timer_index].control & (1 << 7)) && (data & (1 << 7))) {
        timer[timer_index].counter = timer[timer_index].reload_value;
    }

    // set control
    timer[timer_index].control = (timer[timer_index].control & ~0xC7) | (data & 0xC7);
    
    // set enable bits
    // but a counter in count up mode is disabled as the previous timer when it overflows will cause an increment in this timer
    if ((timer[timer_index].control & (1 << 7)) && ((timer_index == 0) || !(timer[timer_index].control & (1 << 2)))) {
        enabled |= (1 << timer_index);
    } else {
        enabled &= ~(1 << timer_index);
    }
}

void Timers::Tick(int cycles) {
    // check each timer and see if they're enabled,
    // if so then increment the counter for that channel
    for (int i = 0; i < 4; i++) {
        if (enabled & (1 << i)) {
            // only increment timer with prescaler selection if 
            // count up timing is disabled
            if (!(timer[i].control & (1 << 2))) {
                // first decrement from cycles_left
                timer[i].cycles_left -= cycles;

                // if the required amount of cycles has passed,
                // then increment the actual counter
                if (timer[i].cycles_left == 0) {
                    timer[i].counter++;
                    // reset the cycles_left back to the original value
                    timer[i].cycles_left += timer[i].cycles_per_count;
                }

                // handle overflow
                if (timer[i].counter > 0xFFFF) {
                    Overflow(i);
                }
            }
        }
    }
}

void Timers::Overflow(int timer_index) {
    timer[timer_index].counter = timer[timer_index].reload_value;

    if (timer[timer_index].control & (1 << 6)) {
        if (arch == 1) {
            core->arm9.SendInterrupt(3 + timer_index);
        } else {
            core->arm7.SendInterrupt(3 + timer_index);
        }
    }
    
    if (timer_index < 3) {
        if ((timer[timer_index + 1].control & (1 << 2)) && (timer[timer_index + 1].control & (1 << 7))) {
            // in count up timing the next timer is incremented on overflow
            if (timer[timer_index + 1].counter == 0xFFFF) {
                Overflow(timer_index + 1);
            } else {
                timer[timer_index + 1].counter++;
            }
        }
    }
}

auto Timers::ReadTMCNT_L(int timer_index) -> u16 {
    return timer[timer_index].counter;
}

auto Timers::ReadTMCNT_H(int timer_index) -> u16 {
    return timer[timer_index].control;
}

auto Timers::ReadTMCNT(int timer_index) -> u32 {
    return (ReadTMCNT_H(timer_index) << 8) | (ReadTMCNT_L(timer_index));
}