#pragma once

#include <array>
#include "nds/scheduler.h"
#include "nds/hardware/irq.h"

namespace nds {

class Timers {
public:
    Timers(Scheduler& scheduler, IRQ& irq);

    void reset();

    u16 read_length(int id) { return update_counter(id); }
    void write_length(int id, u16 value, u32 mask);

    u16 read_control(int id) { return channels[id].control.data; }
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

} // namespace nds