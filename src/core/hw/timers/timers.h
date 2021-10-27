#pragma once

#include <functional>
#include <common/types.h>
#include <core/scheduler/scheduler.h>

class System;

// the NDS has 4 timers for each cpu, so 8 timers in total
class Timers {
public:
    Timers(System& system, int arch);
    void Reset();

    void WriteTMCNT_L(int timer_index, u16 data);
    void WriteTMCNT_H(int timer_index, u16 data);

    u16 ReadTMCNT_L(int timer_index);
    u16 ReadTMCNT_H(int timer_index);
    u32 ReadTMCNT(int timer_index);

    void Overflow(int timer_index);
    void ActivateChannel(int timer_index);
    void DeactivateChannel(int timer_index);

    u16 UpdateCounter(int timer_index);

    struct Timer {
        u16 control;
        // use u32 so we can detect an overflow
        u32 counter;
        u16 reload_value;

        u64 activation_time;
        bool active;

        int shift;
    } timer[4];

    System& system;

    int arch;

    static constexpr int shifts[4] = {0, 6, 8, 10};

    EventType overflow_event[4];
};