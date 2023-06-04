#pragma once

#include "common/types.h"
#include "arm/arch.h"
#include "core/scheduler.h"
#include "core/video/vram.h"
#include "core/video/ppu.h"

namespace core {

class System;

enum class Screen {
    Top,
    Bottom,
};

class VideoUnit {
public:
    VideoUnit(System& system);

    void reset();
    u16 read_dispstat(arm::Arch arch);
    void write_dispstat(arm::Arch arch, u16 value, u32 mask);
    void write_powcnt1(u16 value);

    u32* get_framebuffer(Screen screen);

    VRAM vram;
    PPU ppu_a;
    PPU ppu_b;
    
private:
    void render_scanline_start();
    void render_scanline_end();

    union POWCNT1 {
        struct {
            bool enable_both_lcds : 1;
            bool enable_engine_a : 1;
            bool enable_rendering_engine : 1;
            bool enable_geometry_engine : 1;
            u32 : 5;
            bool enable_engine_b : 1;
            u32 : 5;
            bool display_swap : 1;
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
            u32 : 1;
            bool lyc_setting_msb : 1;
            u8 lyc_setting : 8;
        };

        u16 data;
    };

    EventType scanline_start_event;
    EventType scanline_end_event;

    POWCNT1 powcnt1;
    u16 vcount;
    DISPSTAT dispstat7;
    DISPSTAT dispstat9;
    System& system;
};

} // namespace core