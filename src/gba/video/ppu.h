#pragma once

#include <array>
#include "common/types.h"
#include "common/memory.h"
#include "common/scheduler.h"
#include "gba/hardware/irq.h"

namespace gba {

class System;

class PPU {
public:
    PPU(System& system);

    void reset();

    u16 read_dispcnt() { return dispcnt.data; }
    u16 read_dispstat() { return dispstat.data; }
    u16 read_vcount() { return vcount; }
    u16 read_bgcnt(int id) { return bgcnt[id].data; }

    u32* fetch_framebuffer() { return framebuffer.data(); }

    void write_dispcnt(u16 value, u32 mask);
    void write_dispstat(u16 value, u32 mask);
    void write_bgcnt(int id, u16 value, u32 mask);
    void write_bghofs(int id, u16 value, u32 mask);
    void write_bgvofs(int id, u16 value, u32 mask);

    template <typename T>
    T read_palette_ram(u32 addr) {
        return common::read<T>(palette_ram.data(), addr & 0x3ff);
    }

    template <typename T>
    T read_oam(u32 addr) {
        return common::read<T>(oam.data(), addr & 0x3ff);
    }

    template <typename T>
    void write_palette_ram(u32 addr, T value) {
        u32 masked_addr = addr & 0x3ff;
        if constexpr (sizeof(T) == 1) {
            // for 8-bit writes the value gets duplicated into a 16-bit write
            auto aligned_addr = masked_addr & ~0x1;
            palette_ram[aligned_addr] = value;
            palette_ram[aligned_addr + 1] = value;
        } else {
            common::write<T>(palette_ram.data(), value, addr & 0x3ff);
        }
    }

    template <typename T>
    void write_oam(u32 addr, T value) {
        common::write<T>(oam.data(), value, addr & 0x3ff);
    }

    template <typename T>
    T read_vram(u32 addr) {
        // 0x00000 - 0x18000 is mapped directly
        // 0x18000 - 0x20000 is mapped to 0x10000 - 0x18000
        u32 masked_addr = addr & 0x1ffff;
        if (masked_addr >= 0x18000) {
            masked_addr -= 0x8000;
        }

        return common::read<T>(vram.data(), masked_addr);
    }

    template <typename T>
    void write_vram(u32 addr, T value) {
        // 0x00000 - 0x17fff is mapped directly
        // 0x18000 - 0x1ffff is mapped to 0x10000 - 0x17fff
        u32 masked_addr = addr & 0x1ffff;
        if (masked_addr >= 0x18000) {
            masked_addr -= 0x8000;
        }

        if constexpr (sizeof(T) == 1) {
            // 8-bit writes will be ignored when:
            // writing to the obj region of vram (0x10000 - 0x17fff)
            // when in bitmap mode (0x14000 - 0x17fff)
            u32 bound = dispcnt.bg_mode >= 3 ? 0x14000 : 0x10000;

            // otherwise the 8-bit value gets duplicated into a 16-bit write
            if (masked_addr < bound) {
                auto aligned_addr = masked_addr & ~0x1;
                vram[aligned_addr] = value;
                vram[aligned_addr + 1] = value;
            }
        } else {
            common::write<T>(vram.data(), value, masked_addr);
        }
    }

    std::array<u8, 0x18000> vram;

private:
    void render_scanline_start();
    void render_scanline_end();
    void render_scanline(int line);
    void render_mode0(int id, int line);
    void render_mode3(int id, int line);
    void render_mode4(int id, int line);
    void render_mode5(int id, int line);
    u32 rgb555_to_rgb888(u32 colour);
    void plot(int x, int y, u32 colour);
    void reset_layers();

    void compose_scanline(int line);
    void compose_pixel(int x, int line);

    u8 calculate_enabled_layers(int x, int line);
    bool in_window_bounds(int coord, int start, int end);

    union DISPCNT {
        struct {
            u32 bg_mode : 3;
            u32 : 1;
            bool display_frame_select : 1;
            bool hblank_interval_free : 1;
            bool obj_mapping : 1;
            bool forced_blank : 1;
            bool enable_bg0 : 1;
            bool enable_bg1 : 1;
            bool enable_bg2 : 1;
            bool enable_bg3 : 1;
            bool enable_obj : 1;
            bool enable_win0 : 1;
            bool enable_win1 : 1;
            bool enable_objwin : 1;
        };

        u16 data;
    };

    union DISPSTAT {
        struct {
            bool vblank : 1;
            bool hblank : 1;
            bool lyc : 1;
            bool vblank_irq : 1;
            bool hblank_irq : 1;
            bool lyc_irq : 1;
            u32 : 2;
            u8 lyc_setting : 8;
        };

        u16 data;
    };

    union BGCNT {
        struct {
            u8 priority : 2;
            u8 character_base : 2;
            u16 : 2;
            bool mosaic : 1;
            bool palette_8bpp : 1;
            u8 screen_base : 5;
            bool display_area_overflow : 1;
            u8 size : 2;
        };

        u16 data;
    };

    DISPCNT dispcnt;
    DISPSTAT dispstat;
    std::array<BGCNT, 4> bgcnt;
    std::array<u16, 4> bghofs;
    std::array<u16, 4> bgvofs;
    u16 vcount;
    std::array<u16, 2> winh;
    std::array<u16, 2> winv;
    u16 winin;
    u16 winout;
    std::array<u8, 0x400> palette_ram;
    std::array<u8, 0x400> oam;

    std::array<std::array<u16, 256>, 4> bg_layers;

    common::Scheduler& scheduler;
    IRQ& irq;
    common::EventType scanline_start_event;
    common::EventType scanline_end_event;
    std::array<u32, 240 * 160> framebuffer;

    static constexpr u16 colour_transparent = 0x8000;
};

} // namespace gba