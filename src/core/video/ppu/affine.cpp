#include "common/logger.h"
#include "common/memory.h"
#include "common/bits.h"
#include "core/video/ppu/ppu.h"
#include "core/video/video_unit.h"

namespace core {

void PPU::affine_loop(int id, int width, int height, AffineCallback affine_callback) {
    if (bgcnt[id].mosaic) {
        logger.todo("PPU: handle mosaic");
    }
    
    s32 copy_x = internal_x[id - 2];
    s32 copy_y = internal_y[id - 2];

    for (int pixel = 0; pixel < 256; pixel++) {
        int x = copy_x >> 8;
        int y = copy_y >> 8;

        copy_x += bgpa[id - 2];
        copy_y += bgpc[id - 2];

        if (bgcnt[id].wraparound_ext_palette_slot) {
            x &= width - 1;
            y &= height - 1;
        } else if (x < 0 || x >= width || y < 0 || y >= height) {
            bg_layers[id][pixel] = colour_transparent;
            continue;
        }

        affine_callback(pixel, x, y);
    }

    internal_x[id - 2] += bgpb[id - 2];
    internal_y[id - 2] += bgpd[id - 2];
}

void PPU::render_affine(int id) {
    u32 screen_base = (bgcnt[id].screen_base * 2048) + (dispcnt.screen_base * 65536);
    u32 character_base = (bgcnt[id].character_base * 16384) + (dispcnt.character_base * 65536);
    int size = 128 << bgcnt[id].size;

    affine_loop(id, size, size, [&](int pixel, int x, int y) {
        auto row = y % 8;
        auto column = x % 8;
        u32 screen_addr = screen_base + (y / 8) * (size / 8) + (x / 8);
        u8 tile_number = bg.read<u8>(screen_addr);
        u32 tile_addr = character_base + (tile_number * 64) + (row * 8) + column;
        u8 palette_index = bg.read<u8>(tile_addr);
        
        if (palette_index == 0) {
            bg_layers[id][pixel] = colour_transparent;
        } else {
            bg_layers[id][pixel] = common::read<u16>(palette_ram, (palette_index * 2) & 0x3ff);
        }
    });
}

void PPU::render_extended(int id) {
    if (common::get_bit<7>(bgcnt[id].data)) {
        u32 data_base = bgcnt[id].screen_base * 16384;
        int bitmap_width = extended_dimensions[bgcnt[id].size][0];
        int bitmap_height = extended_dimensions[bgcnt[id].size][1];

        if (common::get_bit<2>(bgcnt[id].data)) {
            // direct colour bitmap
            affine_loop(id, bitmap_width, bitmap_height, [&](int pixel, int x, int y) {
                u32 data_addr = data_base + (y * bitmap_width + x) * 2;
                u16 colour = bg.read<u16>(data_addr);

                if ((colour >> 15) & 0x1) {
                    bg_layers[id][pixel] = colour;
                } else {
                    bg_layers[id][pixel] = colour_transparent;
                }
            });
        } else {
            // 256 colour bitmap
            affine_loop(id, bitmap_width, bitmap_height, [&](int pixel, int x, int y) {
                u32 data_addr = data_base + (y * bitmap_width) + x;
                u8 palette_index = bg.read<u8>(data_addr);

                if (palette_index == 0) {
                    bg_layers[id][pixel] = colour_transparent;
                } else {
                    bg_layers[id][pixel] = common::read<u16>(palette_ram, (palette_index * 2) & 0x3FF);
                }
            });
        }
    } else {
        // 16-bit bgmap entries
        u32 screen_base = (bgcnt[id].screen_base * 2048) + (dispcnt.screen_base * 65536);
        u32 character_base = (bgcnt[id].character_base * 16384) + (dispcnt.character_base * 65536);
        int size = 128 << bgcnt[id].size;

        affine_loop(id, size, size, [&](int pixel, int x, int y) {
            u32 screen_addr = screen_base + ((y / 8) * (size / 8) + (x / 8)) * 2;
            u16 tile_info = bg.read<u16>(screen_addr);
            int tile_number = tile_info & 0x3ff;
            bool horizontal_flip = (tile_info >> 10) & 0x1;
            bool vertical_flip = (tile_info >> 11) & 0x1;
            int palette_number = (tile_info >> 12) & 0xf;

            int row = (vertical_flip ? (y ^ 7) : y) % 8;
            int column = (horizontal_flip ? (x ^ 7) : x) % 8;
            u32 tile_addr = character_base + (tile_number * 64) + (row * 8) + column;
            u8 palette_index = bg.read<u8>(tile_addr);
            
            if (palette_index == 0) {
                bg_layers[id][pixel] = colour_transparent;
            } else if (dispcnt.bg_extended_palette) {
                u32 extended_palette_addr = (id * 8192) + ((palette_number * 256) + palette_index) * 2;
                bg_layers[id][pixel] = bg_extended_palette.read<u16>(extended_palette_addr);
            } else {
                bg_layers[id][pixel] = common::read<u16>(palette_ram, (palette_index * 2) & 0x3ff);
            }
        });
    }
}

void PPU::render_large(int id) {
    int bitmap_width = 512 << (bgcnt[id].size & 0x1);
    int bitmap_height = 1024 >> (bgcnt[id].size & 0x1);

    affine_loop(id, bitmap_width, bitmap_height, [&](int pixel, int x, int y) {
        u32 tile_addr = (y * bitmap_width) + x;
        u8 palette_index = bg.read<u8>(tile_addr);
        
        if (palette_index == 0) {
            bg_layers[id][pixel] = colour_transparent;
        } else {
            bg_layers[id][pixel] = common::read<u16>(palette_ram, (palette_index * 2) & 0x3ff);
        }
    });
}

} // namespace core