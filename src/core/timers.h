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
        // use u32 so we can detect an overflow
        u32 counter;
        u16 reload_value;
        u16 cycles_left;
        u16 cycles_per_count; // specifies the number of ticks that must pass until counter can
        // be incremented
    } timer[4];

    u8 enabled;

    int arch;

    Core* core;


    void WriteTMCNT_L(int timer_index, u16 data);
    void WriteTMCNT_H(int timer_index, u16 data);

    u16 ReadTMCNT_L(int timer_index);
    u16 ReadTMCNT_H(int timer_index);

    u32 ReadTMCNT(int timer_index);

    void Tick(int cycles);
    void Overflow(int timer_index);
};