#pragma once

#include <array>
#include <mutex>
#include "common/types.h"
#include "common/callback.h"
#include "nds/video/vram_region.h"
#include "nds/video/gpu/gpu.h"

namespace nds {

class PPU {
public:
    PPU(GPU& gpu, u8* palette_ram, u8* oam, VRAMRegion& bg, VRAMRegion& obj, VRAMRegion& bg_extended_palette, VRAMRegion& obj_extended_palette, VRAMRegion& lcdc);

    void reset();
    void render_scanline(int line);

    u32 read_dispcnt() const { return dispcnt.data; }
    u16 read_bgcnt(int id) const { return bgcnt[id].data; }
    u16 read_winin() const { return winin; }
    u16 read_winout() const { return winout; }
    u16 read_bldcnt() const { return bldcnt.data; }
    u16 read_bldalpha() const { return bldalpha.data; }
    u16 read_master_bright() const { return master_bright.data; }

    void write_dispcnt(u32 value, u32 mask);

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
    void write_master_bright(u32 value, u32 mask);

    u32* fetch_framebuffer();
    void on_finish_frame();
    
private:
    using AffineCallback = common::Callback<void(int pixel, int x, int y), 40>;

    void render_blank_screen(int line);
    void render_graphics_display(int line);
    void render_vram_display(int line);

    void render_text(int id, int line);

    void affine_loop(int id, int width, int height, AffineCallback affine_callback);
    void render_affine(int id);
    void render_extended(int id);
    void render_large(int id);

    void render_objects(int line);

    u32 rgb555_to_rgb888(u32 colour);
    u32 rgb555_to_rgb666(u32 colour);
    u32 rgb666_to_rgb888(u32 colour);
    void plot(int x, int y, u32 colour);

    void compose_scanline(int line);
    void compose_pixel(int x, int line);
    void compose_pixel_with_special_effects(int x, int line);

    enum SpecialEffect : u16 {
        None = 0,
        AlphaBlending = 1,
        BrightnessIncrease = 2,
        BrightnessDecrease = 3,
    };

    u32 blend(u32 top, u32 bottom, SpecialEffect special_effect);

    using TileRow = std::array<u16, 8>;

    u16 decode_obj_pixel_4bpp(u32 base, int number, int x, int y);
    u16 decode_obj_pixel_8bpp(u32 base, int number, int x, int y);
    TileRow decode_tile_row_4bpp(u32 tile_base, int tile_number, int palette_number, int y, bool horizontal_flip, bool vertical_flip);
    TileRow decode_tile_row_8bpp(u32 tile_base, int tile_number, int palette_number, int y, bool horizontal_flip, bool vertical_flip, int extended_palette_slot);

    u8 calculate_enabled_layers(int x, int line);
    bool in_window_bounds(int coord, int start, int end);

    void begin_scanline();
    void apply_master_brightness(int line);

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

    union MOSAIC {
        struct {
            u8 bg_width : 4;
            u8 bg_height : 4;
            u8 obj_width : 4;
            u8 obj_height : 4;
            u32 : 16;
        };

        u32 data;
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

    union BLDY {
        struct {
            u32 evy : 5;
            u32 : 27;
        };

        u32 data;
    };

    enum BrightnessMode : u32 {
        Disable = 0,
        Increase = 1,
        Decrease = 2,
        Reserved = 3,
    };

    union MASTER_BRIGHT {
        struct {
            u32 factor : 5;
            u32 : 9;
            BrightnessMode mode : 2;
            u32 : 16;
        };

        u32 data;
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

    struct Object {
        int priority;
        u16 colour;
        bool semi_transparent;
    };

    enum class ObjectMode : int {
        Normal = 0,
        SemiTransparent = 1,
        ObjectWindow = 2,
        Bitmap,
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
    MOSAIC mosaic;
    BLDCNT bldcnt;
    BLDY bldy;
    MASTER_BRIGHT master_bright;
    BLDALPHA bldalpha;
    bool line_has_semi_transparent_obj{false};
    
    int mosaic_bg_vertical_counter;

    std::array<u32, 256 * 192> framebuffer;
    std::array<u32, 256 * 192> converted_framebuffer;
    std::mutex converted_framebuffer_mutex;
    std::array<std::array<u16, 256>, 4> bg_layers;
    std::array<Object, 256> obj_buffer;
    
    GPU& gpu;
    u8* palette_ram;
    u8* oam;
    VRAMRegion& bg;
    VRAMRegion& obj;
    VRAMRegion& bg_extended_palette;
    VRAMRegion& obj_extended_palette;
    VRAMRegion& lcdc;
    
    static constexpr int obj_dimensions[4][4][2] = {{{8, 8}, {16, 16}, {32, 32}, {64, 64}}, {{16, 8}, {32, 8}, {32, 16}, {64, 32}}, {{8, 16}, {8, 32}, {16, 32}, {32, 64}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}};
    static constexpr int text_dimensions[4][2] = {{256, 256}, {512, 256}, {256, 512}, {512, 512}};
    static constexpr int extended_dimensions[4][2] = {{128, 128}, {256, 256}, {512, 256}, {512, 512}};
    static constexpr u16 colour_transparent = 0x8000;
};

} // namespace nds