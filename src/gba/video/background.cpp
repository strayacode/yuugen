#include "common/memory.h"
#include "common/bits.h"
#include "gba/video/ppu.h"

namespace gba {

void PPU::render_background(int id, int line) {
    if (bgcnt[id].mosaic) {
        LOG_WARN("handle mosaic");
    }

    int y = (line + bgvofs[id]) % 512;
    u32 screen_base = (bgcnt[id].screen_base * 2048) + ((y / 8) % 32) * 64;
    u32 character_base = bgcnt[id].character_base * 16384;
    int screen_width = bg_dimentions[bgcnt[id].size][0];
    int screen_height = bg_dimentions[bgcnt[id].size][1];

    if (y >= 256 && screen_height == 512) {
        if (screen_width == 512) {
            screen_base += 4096;
        } else {
            screen_base += 2048;
        }
    }

    std::array<u16, 8> pixels;
    for (int tile = 0; tile <= 240; tile += 8) {
        int x = (tile + bghofs[id]) % 512;
        u32 screen_addr = screen_base + ((x / 8) % 32) * 2;

        if (x >= 256 && screen_width == 512) {
            screen_addr += 2048;
        }

        u16 tile_info = common::read<u16>(vram.data(), screen_addr);
        int tile_number = common::get_field<0, 10>(tile_info);
        bool horizontal_flip = common::get_bit<10>(tile_info);;
        bool vertical_flip = common::get_bit<11>(tile_info);
        int palette_number = common::get_field<12, 4>(tile_info);

        if (bgcnt[id].palette_8bpp) {
            pixels = decode_tile_row_8bpp(character_base, tile_number, y, horizontal_flip, vertical_flip);
        } else {
            pixels = decode_tile_row_4bpp(character_base, tile_number, palette_number, y, horizontal_flip, vertical_flip);
        }

        for (int j = 0; j < 8; j++) {
            int offset = tile + j - (x % 8);
            if (offset < 0 || offset >= 240) {
                continue;
            }

            bg_layers[id][offset] = pixels[j];
        }
    }
}

} // namespace gba