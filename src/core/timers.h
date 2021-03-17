#pragma once

#include <util/types.h>
#include <util/log.h>
#include <string.h>

struct Core;


// the NDS has 4 timers for each cpu, so 8 timers in total
struct Timers {
    Timers(Core* core, int arch);
    void Reset();

    struct Timer {
        u16 control;
        u16 counter;
        u16 reload_value;
    } timer[4];

    u8 enabled;

    int arch;

    Core* core;


    void WriteCounter(int timer_index, u16 data);
    void WriteControl(int timer_index, u16 data);
};