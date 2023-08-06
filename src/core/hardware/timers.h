#pragma once

#include <array>
#include "core/scheduler.h"
#include "core/hardware/irq.h"

namespace core {

class Timers {
public:
    Timers(Scheduler& scheduler, IRQ& irq);

    void reset();
    void write_length(int id, u16 value, u32 mask);
    void write_control(int id, u16 value, u32 mask);

private:
    void overflow(int id);
    void activate_channel(int id);
    void deactivate_channel(int id);

    u16 update_counter(int id);

    struct Channel {
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
        };

        Control control;
        u32 counter;
        u32 reload_value;

        u64 activation_timestamp;
        bool active;
        int shift;
    };

    std::array<Channel, 4> channels;
    std::array<EventType, 4> overflow_events;
    Scheduler& scheduler;
    IRQ& irq;

    static constexpr int shifts[4] = {0, 6, 8, 10};
};

} // namespace core