#pragma once

#include <common/types.h>
#include <common/log.h>
#include <common/log_file.h>
#include <string.h>
#include <functional>
#include <memory>

class HW;

// the NDS has 4 timers for each cpu, so 8 timers in total
class Timers {
public:
    Timers(HW* hw, int arch);
    void Reset();

    void WriteTMCNT_L(int timer_index, u16 data);
    void WriteTMCNT_H(int timer_index, u16 data);

    auto ReadTMCNT_L(int timer_index) -> u16;
    auto ReadTMCNT_H(int timer_index) -> u16;

    auto ReadTMCNT(int timer_index) -> u32;

    void Overflow(int timer_index);
    void ActivateChannel(int timer_index);
    void DeactivateChannel(int timer_index);

    int GetEventId(int timer_index);

    struct Timer {
        u16 control;
        // use u32 so we can detect an overflow
        u32 counter;
        u16 reload_value;

        u64 activation_time;
        bool active = false;

        int shift;
    } timer[4];

    HW* hw;

    int arch;

    static constexpr int shifts[4] = {0, 6, 8, 10};

    std::function<void()> OverflowEvent[4];
};