#pragma once

#include <array>
#include "common/types.h"
#include "common/logger.h"
#include "core/video/vram_region.h"

namespace core {

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

    template <typename T>
    T read_arm7(u32 addr) {
        return arm7_vram.read<T>(addr);
    }

    template <typename T>
    void write_arm7(u32 addr, T value) {
        arm7_vram.write<T>(addr, value);
    }

    u8 read_vramstat() { return vramstat; }

    u8 read_vramcnt(Bank bank) {
        logger.log("read vramcnt bank %d %02x\n", static_cast<int>(bank), vramcnt[static_cast<int>(bank)].data);
        return vramcnt[static_cast<int>(bank)].data;
    }
    void write_vramcnt(Bank bank, u8 value);

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

    VRAMRegion<0xa4000> lcdc;
    VRAMRegion<0x80000> bga;
    VRAMRegion<0x40000> obja;
    VRAMRegion<0x20000> bgb;
    VRAMRegion<0x20000> objb;
    VRAMRegion<0x20000> arm7_vram;
    VRAMRegion<0x80000> texture_data;
    VRAMRegion<0x20000> texture_palette;
};

} // namespace core