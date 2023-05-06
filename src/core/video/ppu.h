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

    const u32* get_framebuffer() { return framebuffer.data(); }
    
private:
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
    std::array<u32, 256 * 192> framebuffer;
    VideoUnit& video_unit;
    Engine engine;
};

} // namespace core