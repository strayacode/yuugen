#pragma once

#include <array>
#include "common/types.h"
#include "common/scheduler.h"
#include "arm/arch.h"
#include "arm/memory.h"
#include "nds/hardware/irq.h"

namespace nds {

class DMA {
public:
    DMA(common::Scheduler& scheduler, arm::Memory& memory, IRQ& irq, arm::Arch arch);

    void reset();

    enum Timing : u16 {
        Immediate = 0,
        VBlank = 1,
        HBlank = 2,
        StartOfDisplay = 3,
        MainMemoryDisplay = 4,
        Slot1 = 5,
        Slot2 = 6,
        GXFIFO = 7,
    };

    void trigger(Timing timing);

    u32 read_source(int id) { return channels[id].source; }
    u16 read_length(int id);
    u16 read_control(int id);
    u32 read_dmafill(u32 addr);

    void write_length(int id, u16 value, u32 mask);
    void write_source(int id, u32 value, u32 mask);
    void write_destination(int id, u32 value, u32 mask);
    void write_control(int id, u16 value, u32 mask);
    void write_dmafill(u32 addr, u32 value);

    void set_gxfifo_half_empty(bool value) { gxfifo_half_empty = value; }

private:
    void transfer(int id);

    enum AddressMode : u16 {
        Increment = 0,
        Decrement = 1,
        Fixed = 2,
        Reload = 3,
    };

    struct Channel {
        u32 length;
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
                Timing timing : 3;
                bool irq : 1;
                bool enable : 1;
            };

            u16 data;
        };

        Control control;
    };

    std::array<Channel, 4> channels;
    std::array<u32, 4> dmafill;
    std::array<common::EventType, 4> transfer_events;
    common::Scheduler& scheduler;
    arm::Memory& memory;
    IRQ& irq;
    arm::Arch arch;
    bool gxfifo_half_empty{false};

    static constexpr int adjust_lut[2][4] = {
        {2, -2, 0, 2},
        {4, -4, 0, 4},
    };
};

} // namespace nds