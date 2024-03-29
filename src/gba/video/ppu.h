#pragma once

#include <array>
#include "common/types.h"
#include "common/memory.h"
#include "common/scheduler.h"
#include "gba/hardware/irq.h"
#include "gba/hardware/dma.h"

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
    void write_bgpa(int id, u16 value, u32 mask);
    void write_bgpb(int id, u16 value, u32 mask);
    void write_bgpc(int id, u16 value, u32 mask);
    void write_bgpd(int id, u16 value, u32 mask);
    void write_bgx(int id, u32 value, u32 mask);
    void write_bgy(int id, u32 value, u32 mask);
    void write_winh(int id, u16 value, u32 mask);
    void write_winv(int id, u16 value, u32 mask);
    void write_winin(u16 value, u32 mask);
    void write_winout(u16 value, u32 mask);
    void write_mosaic(u16 value, u32 mask);
    void write_bldcnt(u16 value, u32 mask);
    void write_bldalpha(u16 value, u32 mask);
    void write_bldy(u16 value, u32 mask);

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
    void render_background(int id, int line);
    void render_affine(int id);
    void render_mode3(int id, int line);
    void render_mode4(int id, int line);
    void render_mode5(int id, int line);
    void render_objects(int line);
    u32 rgb555_to_rgb888(u32 colour);
    void plot(int x, int y, u32 colour);
    void reset_layers();

    void compose_scanline(int line);
    void compose_pixel(int x, int line);

    u8 calculate_enabled_layers(int x, int line);
    bool in_window_bounds(int coord, int start, int end);

    using TileRow = std::array<u16, 8>;

    u16 decode_obj_pixel_4bpp(u32 base, int number, int x, int y);
    u16 decode_obj_pixel_8bpp(u32 base, int x, int y);
    TileRow decode_tile_row_4bpp(u32 tile_base, int tile_number, int palette_number, int y, bool horizontal_flip, bool vertical_flip);
    TileRow decode_tile_row_8bpp(u32 tile_base, int tile_number, int y, bool horizontal_flip, bool vertical_flip);

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

    union MOSAIC {
        struct {
            u8 bg_width : 4;
            u8 bg_height : 4;
            u8 obj_width : 4;
            u8 obj_height : 4;
        };

        u16 data;
    };

    enum SpecialEffect : u16 {
        None = 0,
        AlphaBlending = 1,
        BrightnessIncrease = 2,
        BrightnessDecrease = 3,
    };

    union BLDCNT {
        struct {
            u16 first_target : 6;
            SpecialEffect special_effect : 2;
            u16 second_target : 6;
            u16 : 2;
        };

        u16 data;
    };

    union BLDALPHA {
        struct {
            u16 eva : 5;
            u16 : 3;
            u16 evb : 5;
            u16 : 3;
        };

        u16 data;
    };

    union BLDY {
        struct {
            u32 evy : 5;
            u32 : 27;
        };

        u32 data;
    };

    struct Object {
        int priority;
        u16 colour;
    };

    enum class ObjectMode : int {
        Normal = 0,
        SemiTransparent = 1,
        ObjectWindow = 2,
        Reserved,
    };

    DISPCNT dispcnt;
    DISPSTAT dispstat;
    std::array<BGCNT, 4> bgcnt;
    std::array<u16, 4> bghofs;
    std::array<u16, 4> bgvofs;
    std::array<s16, 2> bgpa;
    std::array<s16, 2> bgpb;
    std::array<s16, 2> bgpc;
    std::array<s16, 2> bgpd;
    std::array<s32, 2> bgx;
    std::array<s32, 2> bgy;
    std::array<s32, 2> internal_x;
    std::array<s32, 2> internal_y;
    u16 vcount;
    std::array<u16, 2> winh;
    std::array<u16, 2> winv;
    u16 winin;
    u16 winout;
    MOSAIC mosaic;
    BLDCNT bldcnt;
    BLDALPHA bldalpha;
    BLDY bldy;
    std::array<u8, 0x400> palette_ram;
    std::array<u8, 0x400> oam;

    std::array<std::array<u16, 256>, 4> bg_layers;
    std::array<Object, 256> obj_buffer;

    common::Scheduler& scheduler;
    IRQ& irq;
    DMA& dma;
    common::EventType scanline_start_event;
    common::EventType scanline_end_event;
    std::array<u32, 240 * 160> framebuffer;

    static constexpr int bg_dimentions[4][2] = {{256, 256}, {512, 256}, {256, 512}, {512, 512}};
    static constexpr int obj_dimensions[4][4][2] = {{{8, 8}, {16, 16}, {32, 32}, {64, 64}}, {{16, 8}, {32, 8}, {32, 16}, {64, 32}}, {{8, 16}, {8, 32}, {16, 32}, {32, 64}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}};
    static constexpr u16 colour_transparent = 0x8000;
    static constexpr u32 vram_obj_offset = 0x10000;
};

} // namespace gba