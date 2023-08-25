#pragma once

#include <array>
#include "common/types.h"
#include "common/memory.h"
#include "arm/arch.h"
#include "core/scheduler.h"
#include "core/video/vram.h"
#include "core/video/ppu/ppu.h"
#include "core/video/gpu/gpu.h"
#include "core/hardware/irq.h"

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

    u16 read_powcnt1() { return powcnt1.data; }

    void write_dispstat(arm::Arch arch, u16 value, u32 mask);

    u16 read_vcount() { return vcount; }
    void write_vcount(u16 value, u32 mask);
    void write_powcnt1(u16 value, u32 mask);
    void write_dispcapcnt(u32 value, u32 mask);

    u32* fetch_framebuffer(Screen screen);

    u8* get_palette_ram() { return palette_ram.data(); }
    u8* get_oam() { return oam.data(); }

    template <typename T>
    T read_palette_ram(u32 addr) {
        return common::read<T>(palette_ram.data(), addr & 0x7ff);
    }

    template <typename T>
    T read_oam(u32 addr) {
        return common::read<T>(oam.data(), addr & 0x7ff);
    }

    template <typename T>
    void write_palette_ram(u32 addr, T value) {
        common::write<T>(palette_ram.data(), value, addr & 0x7ff);
    }

    template <typename T>
    void write_oam(u32 addr, T value) {
        common::write<T>(oam.data(), value, addr & 0x7ff);
    }

    VRAM vram;
    PPU ppu_a;
    PPU ppu_b;
    GPU gpu;
    
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

    union DISPCAPCNT {
        struct {
            u32 eva : 5;
            u32 : 3;
            u32 evb : 5;
            u32 : 3;
            u32 vram_write_block : 2;
            u32 vram_write_offset : 2;
            u32 capture_size : 2;
            u32 : 2;
            bool source_a : 1;
            bool source_b : 1;
            u32 vram_read_offset : 2;
            u32 : 1;
            u32 capture_source : 2;
            bool capture_enable : 1;
        };

        u32 data;
    };

    std::array<u8, 0x800> palette_ram;
    std::array<u8, 0x800> oam;

    EventType scanline_start_event;
    EventType scanline_end_event;

    POWCNT1 powcnt1;
    u16 vcount;
    DISPSTAT dispstat7;
    DISPSTAT dispstat9;
    DISPCAPCNT dispcapcnt;
    System& system;
    IRQ& irq7;
    IRQ& irq9;
};

} // namespace core