#pragma once

#include <array>
#include "common/types.h"
#include "common/logger.h"
#include "nds/video/vram_region.h"

namespace nds {

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
        auto region = (addr >> 20) & 0xf;
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
    void write(u32 addr, T value) {
        auto region = (addr >> 20) & 0xf;
        switch (region) {
        case 0x0: case 0x1:
            bga.write<T>(addr, value);
            break;
        case 0x2: case 0x3:
            bgb.write<T>(addr, value);
            break;
        case 0x4: case 0x5:
            obja.write<T>(addr, value);
            break;
        case 0x6: case 0x7:
            objb.write<T>(addr, value);
            break;
        default:
            lcdc.write<T>(addr, value);
            break;
        }
    }

    template <typename T>
    T read_arm7(u32 addr) {
        return arm7_vram.read<T>(addr);
    }

    template <typename T>
    void write_arm7(u32 addr, T value) {
        arm7_vram.write<T>(addr, value);
    }

    u8 read_vramstat() { return vramstat; }

    u8 read_vramcnt(Bank bank) { return vramcnt[static_cast<int>(bank)].data; }
    void write_vramcnt(Bank bank, u8 value);

    VRAMRegion lcdc;
    VRAMRegion bga;
    VRAMRegion obja;
    VRAMRegion bgb;
    VRAMRegion objb;
    VRAMRegion arm7_vram;
    VRAMRegion texture_data;
    VRAMRegion texture_palette;
    VRAMRegion bga_extended_palette;
    VRAMRegion bgb_extended_palette;
    VRAMRegion obja_extended_palette;
    VRAMRegion objb_extended_palette;

private:
    void reset_vram_regions();

    u8 vramstat;

    union VRAMCNT {
        struct {
            u8 mst : 3;
            u8 offset : 2;
            u8 : 2;
            bool enable : 1;
        };

        u8 data;
    };

    std::array<VRAMCNT, 9> vramcnt;

    std::array<u8, 0x20000> bank_a;
    std::array<u8, 0x20000> bank_b;
    std::array<u8, 0x20000> bank_c;
    std::array<u8, 0x20000> bank_d;
    std::array<u8, 0x10000> bank_e;
    std::array<u8, 0x4000> bank_f;
    std::array<u8, 0x4000> bank_g;
    std::array<u8, 0x8000> bank_h;
    std::array<u8, 0x4000> bank_i;
};

} // namespace nds