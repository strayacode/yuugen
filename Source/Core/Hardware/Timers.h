#pragma once

#include "Common/Types.h"
#include "Core/Scheduler.h"
#include "Core/ARM/MMIO.h"

class System;

// the NDS has 4 timers for each cpu, so 8 timers in total
class Timers {
public:
    Timers(System& system, int arch);
    
    void reset();
    void build_mmio(MMIO& mmio);

    void write_counter(int channel, u16 data);
    void write_control(int channel, u16 data);

    u16 read_counter(int channel);
    u16 read_control(int channel);
    
    void overflow(int channel);
    void activate_channel(int channel);
    void deactivate_channel(int channel);

    u16 update_counter(int channel);

    struct Timer {
        union Control {
            struct {
                u8 prescaler : 2;
                bool count_up : 1;
                u32 : 3;
                bool irq : 1;
                bool start : 1;
                u32 : 8;
            };

            u16 data;
        } control;

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