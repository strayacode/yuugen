#pragma once

#include "common/scheduler.h"

namespace gba {

class PPU {
public:
    PPU(common::Scheduler& scheduler);

    void reset();

private:
    void render_scanline_start();
    void render_scanline_end();

    union DISPCNT {
        struct {
            u32 bg_mode : 3;
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

    common::Scheduler& scheduler;
    common::EventType scanline_start_event;
    common::EventType scanline_end_event;
};

} // namespace gba