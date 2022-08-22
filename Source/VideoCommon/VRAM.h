#pragma once

#include <array>
#include "Common/Types.h"
#include "Common/Memory.h"
#include "VideoCommon/VRAMRegion.h"

// a wrapper class to manage vram state
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

    void update_vram_mapping(Bank bank, u8 data);

    template <typename T>
    T read_vram(u32 addr) {
        u8 region = (addr >> 20) & 0xF;

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
    void write_vram(u32 addr, T data) {
        u8 region = (addr >> 20) & 0xF;

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
        T return_value = 0;

        if (get_bank_enabled(vramcnt[2])) {
            if (Common::in_range(0x06000000 + (get_bank_offset(vramcnt[2]) * 0x20000), 0x06020000 + (get_bank_offset(vramcnt[2]) * 0x20000), addr) && (get_bank_mst(vramcnt[2]) == 2)) {
                return Common::read<T>(&bank_c[addr & 0x1FFFF]);
            }
        }

        if (get_bank_enabled(vramcnt[3])) {
            if (Common::in_range(0x06000000 + (get_bank_offset(vramcnt[3]) * 0x20000), 0x06020000 + (get_bank_offset(vramcnt[3]) * 0x20000), addr) && (get_bank_mst(vramcnt[3]) == 2)) {
                return Common::read<T>(&bank_d[addr & 0x1FFFF]);
            }
        }

        return return_value;
    }

    template <typename T>
    void write_arm7(u32 addr, T data) {
        if (get_bank_enabled(vramcnt[2])) {
            if (Common::in_range(0x06000000 + (get_bank_offset(vramcnt[2]) * 0x20000), 0x06020000 + (get_bank_offset(vramcnt[2]) * 0x20000), addr) && (get_bank_mst(vramcnt[2]) == 2)) {
                Common::write<T>(&bank_c[addr & 0x1FFFF], data);
            }
        }

        if (get_bank_enabled(vramcnt[3])) {
            if (Common::in_range(0x06000000 + (get_bank_offset(vramcnt[3]) * 0x20000), 0x06020000 + (get_bank_offset(vramcnt[3]) * 0x20000), addr) && (get_bank_mst(vramcnt[3]) == 2)) {
                Common::write<T>(&bank_d[addr & 0x1FFFF], data);
            }
        }
    }

    template <typename T>
    T read_ext_palette_bga(u32 addr) {
        if (get_bank_enabled(vramcnt[4])) {
            // only lower 32kb are used
            // vram bank e can then hold all 4 8kb slots
            if (Common::in_range(0, 0x8000, addr) && (get_bank_mst(vramcnt[4]) == 4)) {
                return Common::read<T>(&bank_e[addr & 0xFFFF]);
            }
        }

        if (get_bank_enabled(vramcnt[5])) {
            // we will either access slots 0-1 or 2-3 depending on ofs
            u32 offset = get_bank_offset(vramcnt[5]) & 0x1 ? 0x4000 : 0;
            if (Common::in_range(offset, 0x4000, addr) && (get_bank_mst(vramcnt[5]) == 4)) {
                return Common::read<T>(&bank_f[addr & 0x3FFF]);
            }
        }

        if (get_bank_enabled(vramcnt[6])) {
            // we will either access slots 0-1 or 2-3 depending on ofs
            u32 offset = get_bank_offset(vramcnt[6]) & 0x1 ? 0x4000 : 0;
            if (Common::in_range(offset, 0x4000, addr) && (get_bank_mst(vramcnt[6]) == 4)) {
                return Common::read<T>(&bank_g[addr & 0x3FFF]);
            }
        }

        return 0;
    }

    template <typename T>
    T read_ext_palette_bgb(u32 addr) {
        if (get_bank_enabled(vramcnt[7])) {
            // vram bank h can cover all slots 0-3
            if (Common::in_range(0, 0x8000, addr) && (get_bank_mst(vramcnt[7]) == 2)) {
                return Common::read<T>(&bank_h[addr & 0x7FFF]);
            }
        }

        return 0;
    }

    template <typename T>
    T read_ext_palette_obja(u32 addr) {
        // only the lower 8kb of a vram bank is used, since for objs only one 8kb slot is used
        if (get_bank_enabled(vramcnt[5])) {
            if (Common::in_range(0, 0x2000, addr) && (get_bank_mst(vramcnt[5]) == 5)) {
                return Common::read<T>(&bank_f[addr & 0x1FFF]);
            }
        }

        if (get_bank_enabled(vramcnt[6])) {
            // we will either access slots 0-1 or 2-3 depending on ofs
            if (Common::in_range(0, 0x2000, addr) && (get_bank_mst(vramcnt[6]) == 5)) {
                return Common::read<T>(&bank_g[addr & 0x1FFF]);
            }
        }

        return 0;
    }

    template <typename T>
    T read_ext_palette_objb(u32 addr) {
        // only the lower 8kb of a vram bank is used, since for objs only one 8kb slot is used
        if (get_bank_enabled(vramcnt[8])) {
            if (Common::in_range(0, 0x2000, addr) && (get_bank_mst(vramcnt[8]) == 3)) {
                return Common::read<T>(&bank_i[addr & 0x1FFF]);
            }
        }

        return 0;
    }

    template <typename T>
    T read_texture_data(u32 addr) {
        return texture_data.read<T>(addr);
    }

    template <typename T>
    T read_texture_palette(u32 addr) {
        return texture_palette.read<T>(addr);
    }

    std::array<u8, 9> vramcnt = {};
    u8 vramstat = 0;

private:
    void reset_vram_mapping();

    int get_bank_mst(u8 vramcnt);
    int get_bank_offset(u8 vramcnt);
    bool get_bank_enabled(u8 vramcnt);

    // banks which hold the raw vram data
    std::array<u8, 0x20000> bank_a = {};
    std::array<u8, 0x20000> bank_b = {};
    std::array<u8, 0x20000> bank_c = {};
    std::array<u8, 0x20000> bank_d = {};
    std::array<u8, 0x10000> bank_e = {};
    std::array<u8, 0x4000> bank_f = {};
    std::array<u8, 0x4000> bank_g = {};
    std::array<u8, 0x8000> bank_h = {};
    std::array<u8, 0x4000> bank_i = {};
    
    // regions which get mapped to banks
    VRAMRegion<656> lcdc;
    VRAMRegion<512> bga;
    VRAMRegion<256> obja;
    VRAMRegion<128> bgb;
    VRAMRegion<128> objb;
    VRAMRegion<128> arm7_vram;
    VRAMRegion<512> texture_data;
    VRAMRegion<128> texture_palette;
};