#pragma once

#include <array>
#include "common/types.h"
#include "core/nds/video/vram_region.h"

namespace core::nds {

class VRAM {
public:
    void reset();

    enum class Bank {
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
    };

    void write_vramcnt(Bank bank, u8 value);

private:
    void reset_vram_regions();

    std::array<u8, 9> vramcnt;

    std::array<u8, 0x20000> bank_a;
    std::array<u8, 0x20000> bank_b;
    std::array<u8, 0x20000> bank_c;
    std::array<u8, 0x20000> bank_d;
    std::array<u8, 0x10000> bank_e;
    std::array<u8, 0x4000> bank_f;
    std::array<u8, 0x4000> bank_g;
    std::array<u8, 0x8000> bank_h;
    std::array<u8, 0x4000> bank_i;

    VRAMRegion<0xa4000> lcdc;
    VRAMRegion<0x80000> bga;
    VRAMRegion<0x40000> obja;
    VRAMRegion<0x20000> bgb;
    VRAMRegion<0x20000> objb;
    VRAMRegion<0x20000> arm7_vram;
    VRAMRegion<0x80000> texture_data;
    VRAMRegion<0x20000> texture_palette;
};

} // namespace core::nds