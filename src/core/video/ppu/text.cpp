#include "common/logger.h"
#include "common/bits.h"
#include "core/video/ppu/ppu.h"
#include "core/video/video_unit.h"

namespace core {

void PPU::render_text(int bg, int line) {
    if (bgcnt[bg].mosaic) {
        logger.todo("PPU: handle mosaic");
    }

    int y = (line + bgvofs[bg]) % 512;
    u32 screen_base = vram_addr + (dispcnt.screen_base * 65536) + (bgcnt[bg].screen_base * 2048) + ((y / 8) % 32) * 64;
    u32 character_base = vram_addr + (dispcnt.character_base * 65536) + (bgcnt[bg].character_base * 16384);
    int extended_palette_slot = bg | (bgcnt[bg].wraparound_ext_palette_slot * 2);
    int screen_width = text_dimensions[bgcnt[bg].size][0];
    int screen_height = text_dimensions[bgcnt[bg].size][1];

    if (y >= 256 && screen_height == 512) {
        if (screen_width == 512) {
            screen_base += 4096;
        } else {
            screen_base += 2048;
        }
    }

    std::array<u16, 8> pixels;
    for (int tile = 0; tile <= 256; tile += 8) {
        int x = (tile + bghofs[bg]) % 512;
        u32 screen_addr = screen_base + ((x / 8) % 32) * 2;

        if (x >= 256 && screen_width == 512) {
            screen_addr += 2048;
        }

        u16 tile_info = video_unit.vram.read<u16>(screen_addr);
        int tile_number = common::get_field<0, 10>(tile_info);
        bool horizontal_flip = common::get_bit<10>(tile_info);;
        bool vertical_flip = common::get_bit<11>(tile_info);
        int palette_number = common::get_field<12, 4>(tile_info);

        if (bgcnt[bg].palette_8bpp) {
            pixels = decode_tile_row_8bpp(character_base, tile_number, palette_number, y, horizontal_flip, vertical_flip, extended_palette_slot);
        } else {
            pixels = decode_tile_row_4bpp(character_base, tile_number, palette_number, y, horizontal_flip, vertical_flip);
        }

        for (int j = 0; j < 8; j++) {
            int offset = tile + j - (x % 8);
            if (offset < 0 || offset >= 256) {
                continue;
            }

            bg_layers[bg][offset] = pixels[j];
        }
    }
}

} // namespace core