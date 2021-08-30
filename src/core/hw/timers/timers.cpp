#include <core/hw/timers/timers.h>
#include <core/hw/hw.h>

Timers::Timers(HW* hw, int arch) : hw(hw), arch(arch) {
    for (int i = 0; i < 4; i++) {
        OverflowEvent[i] = std::bind(&Timers::Overflow, this, i);
    }
}

void Timers::Reset() {
    for (int i = 0; i < 4; i++) {
        memset(&timer[i], 0, sizeof(Timer));
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
        hw->cpu_core[arch]->SendInterrupt(3 + timer_index);
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
    // this function is called when a timer is placed on the scheduler

    // make sure the timer is now marked as active
    timer[timer_index].active = true;

    // store the activation time
    timer[timer_index].activation_time = hw->scheduler.GetCurrentTime();

    // determine the delay of the event
    // for this we must see how many cycles are left internally until the
    // timer overflows, and multiply that by the prescalar value
    u64 delay = (0x10000 - timer[timer_index].counter) << timer[timer_index].shift;

    // now add the event
    hw->scheduler.AddWithId(delay, GetEventId(timer_index), OverflowEvent[timer_index]);
}

void Timers::DeactivateChannel(int timer_index) {
    // this function is called when a timer is altered while a timer is on the scheduler

    // mark the timer as not active anymore
    timer[timer_index].active = false;

    // update the counter of the timer
    timer[timer_index].counter += (hw->scheduler.GetCurrentTime() - timer[timer_index].activation_time) >> timer[timer_index].shift;

    if (timer[timer_index].counter >= 0x10000) {
        log_fatal("handle");
    }

    // cancel the event
    hw->scheduler.Cancel(GetEventId(timer_index));
}

// TODO: inline small functions later
int Timers::GetEventId(int timer_index) {
    int id = TimerEvent + (arch * 4) + timer_index;

    return id;
}

auto Timers::ReadTMCNT_L(int timer_index) -> u16 {
    // we need to now update counter to be relative to the time that has passed since
    // the timer was placed on the scheduler
    if (timer[timer_index].active) {
        timer[timer_index].counter = (hw->scheduler.GetCurrentTime() - timer[timer_index].activation_time) >> timer[timer_index].shift;
    }
    
    return timer[timer_index].counter;
}

auto Timers::ReadTMCNT_H(int timer_index) -> u16 {
    return timer[timer_index].control;
}

auto Timers::ReadTMCNT(int timer_index) -> u32 {
    return (ReadTMCNT_H(timer_index) << 16) | (ReadTMCNT_L(timer_index));
}