#pragma once

#include <array>
#include "common/types.h"
#include "arm/cpu.h"

namespace gba {

class IRQ {
public:
    IRQ(std::unique_ptr<arm::CPU>& cpu);

    enum class Source {
        VBlank = 0,
        HBlank = 1,
        VCounter = 2,
        Timer0 = 3,
        Timer1 = 4,
        Timer2 = 5,
        Timer3 = 6,
        Serial = 7,
        DMA0 = 8,
        DMA1 = 9,
        DMA2 = 10,
        DMA3 = 11,
        Keypad = 12,
        Gamepak = 13,
    };

    void reset();
    void raise(Source source);

    u16 read_ime() { return ime; }
    u16 read_ie() { return ie; }
    u16 read_irf() { return irf; }

    void write_ime(u16 value, u32 mask);
    void write_ie(u16 value, u32 mask);
    void write_irf(u16 value, u32 mask);

private:
    u16 ime;
    u16 ie;
    u16 irf;
    std::unique_ptr<arm::CPU>& cpu;
};

} // namespace gba