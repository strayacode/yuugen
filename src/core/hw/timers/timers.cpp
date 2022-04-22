#include "Common/Log.h"
#include "core/hw/timers/timers.h"
#include "core/core.h"

Timers::Timers(System& system, int arch) : system(system), arch(arch) {
    
}

void Timers::Reset() {
    for (int i = 0; i < 4; i++) {
        memset(&timer[i], 0, sizeof(Timer));
    }

    for (int i = 0; i < 4; i++) {
        std::string name;
        name += arch == 1 ? "ARM9" : "ARM7";
        name += "TimerOverflow" + std::to_string(i);
        overflow_event[i] = system.scheduler.RegisterEvent(name, [this, i]() {
            Overflow(i);
        });
    }
}

void Timers::WriteTMCNT_L(int timer_index, u16 data) {
    timer[timer_index].reload_value = data;
}

void Timers::WriteTMCNT_H(int timer_index, u16 data) {
    // if count up timing is enabled the prescalar value is ignored so instead the timer increments when the previous counter overflows? hmm
    // count up timing cant be used on timer 0 too

    // if a timer was currently already scheduled,
    // we must cancel it as we are now altering the timer
    if (timer[timer_index].active) {
        DeactivateChannel(timer_index);
    }
    
    // set the timer shift
    timer[timer_index].shift = shifts[data & 0x3];

    // if the timer has gone from disabled to enabled then reload tmcnt_l with the reload value
    if (!(timer[timer_index].control & (1 << 7)) && (data & (1 << 7))) {
        timer[timer_index].counter = timer[timer_index].reload_value;
    }

    // set control
    timer[timer_index].control = (timer[timer_index].control & ~0xC7) | (data & 0xC7);

    // a timer in count up mode is disabled as the previous timer when it overflows will cause an increment in this timer
    if ((timer[timer_index].control & (1 << 7)) && ((timer_index == 0) || !(timer[timer_index].control & (1 << 2)))) {
        // this signifies that the channel is not in count up mode
        // so activate the channel
        ActivateChannel(timer_index);
    }
}

void Timers::Overflow(int timer_index) {
    timer[timer_index].counter = timer[timer_index].reload_value;

    if (timer[timer_index].control & (1 << 6)) {
        switch (timer_index) {
        case 0:
            system.cpu_core[arch].SendInterrupt(InterruptType::Timer0);
            break;
        case 1:
            system.cpu_core[arch].SendInterrupt(InterruptType::Timer1);
            break;
        case 2:
            system.cpu_core[arch].SendInterrupt(InterruptType::Timer2);
            break;
        case 3:
            system.cpu_core[arch].SendInterrupt(InterruptType::Timer3);
            break;
        }
    }

    // reactivate the timer if it's not in count up mode
    if ((timer_index == 0) || !(timer[timer_index].control & (1 << 2))) {
        ActivateChannel(timer_index);
    }
    
    if (timer_index < 3) {
        if ((timer[timer_index + 1].control & (1 << 2)) && (timer[timer_index + 1].control & (1 << 7))) {
            // in count up mode the next timer is incremented on overflow
            if (++timer[timer_index + 1].counter == 0x10000) {
                Overflow(timer_index + 1);
            }
        }
    }
}

void Timers::ActivateChannel(int timer_index) {
    timer[timer_index].active = true;
    timer[timer_index].activation_time = system.scheduler.GetCurrentTime();

    // determine the delay of the event
    // for this we must see how many cycles are left internally until the
    // timer overflows, and multiply that by the prescalar value
    u64 delay = (0x10000 - timer[timer_index].counter) << timer[timer_index].shift;

    // now add the event
    system.scheduler.AddEvent(delay, &overflow_event[timer_index]);
}

void Timers::DeactivateChannel(int timer_index) {
    timer[timer_index].counter = UpdateCounter(timer_index);
    timer[timer_index].active = false;

    if (timer[timer_index].counter >= 0x10000) {
        log_fatal("handle");
    }

    system.scheduler.CancelEvent(&overflow_event[timer_index]);
}

u16 Timers::ReadTMCNT_L(int timer_index) {
    return UpdateCounter(timer_index);
}

u16 Timers::ReadTMCNT_H(int timer_index) {
    return timer[timer_index].control;
}

u32 Timers::ReadTMCNT(int timer_index) {
    return (ReadTMCNT_H(timer_index) << 16) | (ReadTMCNT_L(timer_index));
}

u16 Timers::UpdateCounter(int timer_index) {
    u16 counter = timer[timer_index].counter;
    u16 change = (system.scheduler.GetCurrentTime() - timer[timer_index].activation_time) >> timer[timer_index].shift;
    
    if (timer[timer_index].active) {
        return counter + change;
    } else {
        return counter;
    }
}