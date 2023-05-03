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

    template <typename T>
    T read(u32 addr) {
        int region = (addr >> 20) & 0xf;
        switch (region) {
        case 0x0: case 0x1:
            return bga.read<T>(addr);
        case 0x2: case 0x3:
            return bgb.read<T>(addr);
        case 0x4: case 0x5:
            return obja.read<T>(addr);
        case 0x6: case 0x7:
            return objb.read<T>(addr);
        default:
            return lcdc.read<T>(addr);
        }
    }

    template <typename T>
    void write(u32 addr, T data) {
        int region = (addr >> 20) & 0xf;

        switch (region) {
        case 0x0: case 0x1:
            bga.write<T>(addr, data);
            break;
        case 0x2: case 0x3:
            bgb.write<T>(addr, data);
            break;
        case 0x4: case 0x5:
            obja.write<T>(addr, data);
            break;
        case 0x6: case 0x7:
            objb.write<T>(addr, data);
            break;
        default:
            lcdc.write<T>(addr, data);
            break;
        }
    }

    void write_vramcnt(Bank bank, u8 value);

private:
    void reset_vram_regions();
    bool is_bank_enabled(u8 vramcnt);
    int get_bank_offset(u8 vramcnt);
    int get_bank_mst(u8 vramcnt);

    u8 vramstat;
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