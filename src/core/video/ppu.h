#pragma once

#include <array>
#include "common/types.h"

namespace core {

class VideoUnit;

enum class Engine {
    A,
    B,
};

class PPU {
public:
    PPU(VideoUnit& video_unit, Engine engine);

    void reset();
    void render_scanline(int line);
    void write_dispcnt(u32 value, u32 mask);

    void write_bgcnt(int index, u16 value, u32 mask);
    void write_bghofs(int index, u16 value, u32 mask);
    void write_bgvofs(int index, u16 value, u32 mask);
    void write_bgpa(int index, u16 value, u32 mask);
    void write_bgpb(int index, u16 value, u32 mask);
    void write_bgpc(int index, u16 value, u32 mask);
    void write_bgpd(int index, u16 value, u32 mask);
    void write_bgx(int index, u16 value, u32 mask);
    void write_bgy(int index, u16 value, u32 mask);

    u32* get_framebuffer() { return framebuffer.data(); }
    
private:
    void render_blank_screen(int line);
    void render_graphics_display(int line);
    void render_vram_display(int line);

    u32 rgb555_to_rgb888(u32 colour);
    void render_pixel(int x, int y, u32 colour);

    union DISPCNT {
        struct {
            u32 bgmode : 3;
            bool bg0_3d : 1;
            bool tile_obj_mapping : 1;
            bool bitmap_obj_2d : 1;
            bool bitmap_obj_mapping : 1;
            bool forced_blank : 1;
            bool enable_bg0 : 1;
            bool enable_bg1 : 1;
            bool enable_bg2 : 1;
            bool enable_bg3 : 1;
            bool enable_obj : 1;
            bool enable_win0 : 1;
            bool enable_win1 : 1;
            bool enable_objwin : 1;
            u32 display_mode : 2;
            u32 vram_block : 2;
            u32 tile_obj_1d_boundary : 2;
            bool bitmap_obj_1d_boundary : 1;
            bool obj_during_hblank : 1;
            u32 character_base : 3;
            u32 screen_base : 3;
            bool bg_extended_palette : 1;
            bool obj_extended_palette : 1;
        };

        u32 data;
    };

    DISPCNT dispcnt;
    std::array<u16, 4> bgcnt;
    std::array<u16, 4> bghofs;
    std::array<u16, 4> bgvofs;
    std::array<u16, 2> bgpa;
    std::array<u16, 2> bgpb;
    std::array<u16, 2> bgpc;
    std::array<u16, 2> bgpd;
    std::array<u32, 2> bgx;
    std::array<u32, 2> bgy;
    std::array<u32, 2> internal_x;
    std::array<u32, 2> internal_y;
    std::array<u16, 2> winh;
    std::array<u16, 2> winv;
    std::array<u32, 256 * 192> framebuffer;
    VideoUnit& video_unit;
    Engine engine;
};

} // namespace core