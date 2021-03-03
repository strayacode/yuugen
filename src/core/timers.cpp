#include <core/core.h>
#include <core/timers.h>

Timers::Timers(Core* core, int arch) : core(core), arch(arch) {

}

void Timers::Reset() {
    for (int i = 0; i < 4; i++) {
        memset(&timer[i], 0, sizeof(Timer));
    }

    enabled = 0;
}

void Timers::WriteCounter(int timer_index, u16 data) {
    timer[timer_index].reload_value = data;
}

void Timers::WriteControl(int timer_index, u16 data) {
    // if count up timing is enabled the prescalar value is ignored so instead the timer increments when the previous counter overflows? hmm
    // count up timing cant be used on timer 0 too
    
    // set bits 0..1 to the number of cycles left before tmcnt_l can increment
    // switch (data & 0x3) {
    // case 0:
    //     cycles_left[index] = 1;
    // case 1:
    //     cycles_left[index] = 64;
    // case 2:
    //     cycles_left[index] = 256;
    // case 3:
    //     cycles_left[index] = 1024;
    // }


    // if the timer has gone from disabled to enabled then reload tmcnt_l with the reload value
    if (!(timer[timer_index].control && (1 << 7)) && (data & (1 << 7))) {
        timer[timer_index].counter = timer[timer_index].reload_value;
    }

    // set control
    timer[timer_index].control = (timer[timer_index].control & ~0xC7) | (data & 0xC7);
    
    // set enable bits
    // but a counter in count up mode is disabled as the previous timer when it overflows will cause an increment in this timer
    if ((timer[timer_index].control && (1 << 7)) && ((timer_index == 0) || !(timer[timer_index].control & (1 << 2)))) {
        log_fatal("lol we should tick stuff now");
        enabled |= (1 << timer_index);
    } else {
        enabled &= ~(1 << timer_index);
    }
}