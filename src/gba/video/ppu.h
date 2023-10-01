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

    u32* fetch_framebuffer() { return framebuffer.data(); }

    void write_dispcnt(u16 value, u32 mask);

    template <typename T>
    T read_palette_ram(u32 addr) {
        return common::read<T>(palette_ram.data(), addr & 0x3ff);
    }

    template <typename T>
    void write_palette_ram(u32 addr, T value) {
        common::write<T>(palette_ram.data(), value, addr & 0x3ff);
    }

    std::array<u8, 0x18000> vram;

private:
    void render_scanline_start();
    void render_scanline_end();
    void render_scanline(int line);
    void render_mode4(int line);
    u32 rgb555_to_rgb888(u32 colour);
    void plot(int x, int y, u32 colour);

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

    DISPCNT dispcnt;
    DISPSTAT dispstat;
    u16 vcount;
    std::array<u8, 0x400> palette_ram;

    common::Scheduler& scheduler;
    IRQ& irq;
    common::EventType scanline_start_event;
    common::EventType scanline_end_event;
    std::array<u32, 240 * 160> framebuffer;
};

} // namespace gba