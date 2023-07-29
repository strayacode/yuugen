#pragma once

#include <array>
#include "common/types.h"
#include "common/callback.h"

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

    u32 read_dispcnt() { return dispcnt.data; }
    u16 read_bgcnt(int index) { return bgcnt[index].data; }

    void write_dispcnt(u32 value, u32 mask);

    void write_bgcnt(int index, u16 value, u32 mask);
    void write_bghofs(int index, u16 value, u32 mask);
    void write_bgvofs(int index, u16 value, u32 mask);
    void write_bgpa(int index, u16 value, u32 mask);
    void write_bgpb(int index, u16 value, u32 mask);
    void write_bgpc(int index, u16 value, u32 mask);
    void write_bgpd(int index, u16 value, u32 mask);
    void write_bgx(int index, u32 value, u32 mask);
    void write_bgy(int index, u32 value, u32 mask);
    void write_winh(int index, u16 value, u32 mask);
    void write_winv(int index, u16 value, u32 mask);
    void write_winin(u16 value, u32 mask);
    void write_winout(u16 value, u32 mask);
    void write_mosaic(u16 value, u32 mask);
    void write_bldcnt(u16 value, u32 mask);
    void write_bldalpha(u16 value, u32 mask);
    void write_bldy(u16 value, u32 mask);

    u32* get_framebuffer() { return framebuffer.data(); }
    
private:
    using AffineCallback = common::Callback<void(int pixel, int x, int y), 40>;

    void render_blank_screen(int line);
    void render_graphics_display(int line);
    void render_vram_display(int line);

    void render_text(int bg, int line);

    void affine_loop(int bg, int width, int height, AffineCallback affine_callback);
    void render_affine(int bg, int line);
    void render_extended(int bg, int line);
    void render_large(int bg, int line);

    void render_objects(int line);

    u32 rgb555_to_rgb888(u32 colour);
    void render_pixel(int x, int y, u32 colour);

    void compose_scanline(int line);
    void compose_pixel(int x, int line);

    using TileRow = std::array<u16, 8>;

    u16 decode_obj_pixel_4bpp(u32 base, int number, int x, int y);
    u16 decode_obj_pixel_8bpp(u32 base, int number, int x, int y);
    TileRow decode_tile_row_4bpp(u32 tile_base, int tile_number, int palette_number, int y, bool horizontal_flip, bool vertical_flip);
    TileRow decode_tile_row_8bpp(u32 tile_base, int tile_number, int palette_number, int y, bool horizontal_flip, bool vertical_flip, int extended_palette_slot);

    u8 calculate_enabled_layers(int x, int line);
    bool in_window_bounds(int coord, int start, int end);

    void reset_layers();

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

    union BGCNT {
        struct {
            u8 priority : 2;
            u8 character_base : 4;
            bool mosaic : 1;
            bool palette_8bpp : 1;
            u8 screen_base : 5;
            bool wraparound_ext_palette_slot : 1;
            u8 size : 2;
        };

        u16 data;
    };

    union BLDCNT {
        struct {
            bool bg0_first_target : 1;
            bool bg1_first_target : 1;
            bool bg2_first_target : 1;
            bool bg3_first_target : 1;
            bool obj_first_target : 1;
            bool bd_first_target : 1;
            bool special_effects : 2;
            bool bg0_second_target : 1;
            bool bg1_second_target : 1;
            bool bg2_second_target : 1;
            bool bg3_second_target : 1;
            bool obj_second_target : 1;
            bool bd_second_target : 1;
            u16 : 2;
        };

        u16 data;
    };

    DISPCNT dispcnt;
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
    std::array<u16, 2> winh;
    std::array<u16, 2> winv;
    u16 winin;
    u16 winout;
    u32 mosaic;
    BLDCNT bldcnt;
    u16 bldalpha;
    u32 bldy;
    u16 master_bright;

    std::array<u32, 256 * 192> framebuffer;
    std::array<std::array<u16, 256>, 4> bg_layers;
    std::array<int, 256> obj_priority;
    std::array<u16, 256> obj_colour;

    u8* palette_ram;
    u8* oam;
    u32 vram_addr;
    u32 obj_addr;
    VideoUnit& video_unit;
    Engine engine;

    static constexpr int obj_dimensions[3][4][2] = {{{8, 8}, {16, 16}, {32, 32}, {64, 64}}, {{16, 8}, {32, 8}, {32, 16}, {64, 32}}, {{8, 16}, {8, 32}, {16, 32}, {32, 64}}};
    static constexpr int text_dimensions[4][2] = {{256, 256}, {512, 256}, {256, 512}, {512, 512}};
    static constexpr int extended_dimensions[4][2] = {{128, 128}, {256, 256}, {512, 256}, {512, 512}};
    static constexpr u16 colour_transparent = 0x8000;
};

} // namespace core