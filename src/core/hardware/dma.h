#pragma once

#include <array>
#include "common/types.h"
#include "arm/arch.h"
#include "arm/memory.h"
#include "core/hardware/irq.h"

namespace core {

class DMA {
public:
    DMA(arm::Memory& memory, IRQ& irq, arm::Arch arch);

    void reset();

    // TODO: in read_control read 5 bits from length

    void write_length(int index, u16 value, u32 mask);
    void write_source(int index, u32 value, u32 mask);
    void write_destination(int index, u32 value, u32 mask);
    void write_control(int index, u16 value, u32 mask);

private:
    void transfer(int index);

    enum AddressMode : int {
        Increment = 0,
        Decrement = 1,
        Fixed = 2,
        Reload = 3,
    };

    enum Timing : int {
        Immediate = 0,
        VBlank = 1,
        HBlank = 2,
        StartOfDisplay = 3,
        MainMemoryDisplay = 4,
        Slot1 = 5,
        Slot2 = 6,
        GXFIFO = 7,
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
    std::array<u32, 4> dmafill;
    arm::Memory& memory;
    IRQ& irq;
    arm::Arch arch;

    static constexpr int adjust_lut[2][4] = {
        {2, -2, 0, 2},
        {4, -4, 0, 4},
    };
};

} // namespace core