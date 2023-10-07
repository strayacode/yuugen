#pragma once

#include <array>
#include "common/types.h"
#include "common/scheduler.h"
#include "arm/memory.h"
#include "gba/hardware/irq.h"

namespace gba {

class DMA {
public:
    DMA(common::Scheduler& scheduler, arm::Memory& memory, IRQ& irq);

    void reset();

    enum Timing : u16 {
        Immediate = 0,
        VBlank = 1,
        HBlank = 2,
        Special = 3,
    };

    void trigger(Timing timing);

    u32 read_source(int id) { return channels[id].source; }
    u16 read_length(int id);
    u16 read_control(int id);

    void write_length(int id, u16 value, u32 mask);
    void write_source(int id, u32 value, u32 mask);
    void write_destination(int id, u32 value, u32 mask);
    void write_control(int id, u16 value, u32 mask);
    
private:
    void transfer(int id);

    enum AddressMode : u16 {
        Increment = 0,
        Decrement = 1,
        Fixed = 2,
        Reload = 3,
    };

    struct Channel {
        u16 length;
        u32 source;
        u32 internal_source;
        u32 destination;
        u32 internal_destination;
        u32 internal_length;

        union Control {
            struct {
                u16 : 5;
                AddressMode destination_control : 2;
                AddressMode source_control : 2;
                bool repeat : 1;
                bool transfer_words : 1;
                bool gamepak_drq : 1;
                Timing timing : 2;
                bool irq : 1;
                bool enable : 1;
            };

            u16 data;
        };

        Control control;
    };

    std::array<Channel, 4> channels;
    std::array<common::EventType, 4> transfer_events;
    common::Scheduler& scheduler;
    arm::Memory& memory;
    IRQ& irq;
    
    static constexpr int adjust_lut[2][4] = {
        {2, -2, 0, 2},
        {4, -4, 0, 4},
    };
};

} // namespace gba