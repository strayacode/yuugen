#include "common/logger.h"
#include "common/bits.h"
#include "nds/video/ppu/ppu.h"
#include "nds/video/video_unit.h"

namespace nds {

void PPU::render_text(int id, int line) {
    logger.debug("render text bg %d line %d", id, line);
    logger.debug("bgcnt %04x", bgcnt[id].data);
    // apply vertical mosaic
    if (bgcnt[id].mosaic) {
        line -= mosaic_bg_vertical_counter;
    }

    int y = (line + bgvofs[id]) % 512;
    u32 screen_base = (dispcnt.screen_base * 65536) + (bgcnt[id].screen_base * 2048) + ((y / 8) % 32) * 64;
    u32 character_base = (dispcnt.character_base * 65536) + (bgcnt[id].character_base * 16384);
    int extended_palette_slot = id | (bgcnt[id].wraparound_ext_palette_slot * 2);
    int screen_width = text_dimensions[bgcnt[id].size][0];
    int screen_height = text_dimensions[bgcnt[id].size][1];

    if (y >= 256 && screen_height == 512) {
        if (screen_width == 512) {
            screen_base += 4096;
        } else {
            screen_base += 2048;
        }
    }

    std::array<u16, 8> pixels;
    for (int tile = 0; tile <= 256; tile += 8) {
        int x = (tile + bghofs[id]) % 512;
        u32 screen_addr = screen_base + ((x / 8) % 32) * 2;

        if (x >= 256 && screen_width == 512) {
            screen_addr += 2048;
        }

        u16 tile_info = bg.read<u16>(screen_addr);
        int tile_number = common::get_field<0, 10>(tile_info);
        bool horizontal_flip = common::get_bit<10>(tile_info);;
        bool vertical_flip = common::get_bit<11>(tile_info);
        int palette_number = common::get_field<12, 4>(tile_info);

        logger.debug("render at x %d 8bpp = %d", x, bgcnt[id].palette_8bpp);

        if (bgcnt[id].palette_8bpp) {
            pixels = decode_tile_row_8bpp(character_base, tile_number, palette_number, y, horizontal_flip, vertical_flip, extended_palette_slot);
        } else {
            pixels = decode_tile_row_4bpp(character_base, tile_number, palette_number, y, horizontal_flip, vertical_flip);
        }

        for (int j = 0; j < 8; j++) {
            int offset = tile + j - (x % 8);
            if (offset < 0 || offset >= 256) {
                continue;
            }

            bg_layers[id][offset] = pixels[j];
        }
    }

    // apply horizontal mosaic
    if (bgcnt[id].mosaic && mosaic.bg_width != 0) {
        logger.debug("apply horizontal mosaic");
        int mosaic_bg_horizontal_counter = 0;

        for (int i = 0; i < 256; i++) {
            bg_layers[id][i] = bg_layers[id][i - mosaic_bg_horizontal_counter];
            if (mosaic_bg_horizontal_counter == mosaic.bg_width) {
                mosaic_bg_horizontal_counter = 0;
            } else {
                mosaic_bg_horizontal_counter++;
            }
        }
    }
}

} // namespace nds