#include "common/memory.h"
#include "gba/video/ppu.h"

namespace gba {

PPU::TileRow PPU::decode_tile_row_4bpp(u32 tile_base, int tile_number, int palette_number, int y, bool horizontal_flip, bool vertical_flip) {
    TileRow pixels;
    int row = (vertical_flip ? (y ^ 7) : y) % 8;
    u32 tile_addr = tile_base + (tile_number * 32) + (row * 4);
    u32 palette_indices = common::read<u32>(vram.data(), tile_addr);

    for (int x = 0; x < 8; x++) {
        int column = horizontal_flip ? (x ^ 7) : x;
        int palette_index = palette_indices & 0xf;
        u32 palette_addr = (palette_number * 32) + (palette_index * 2);

        u16 colour = (palette_index == 0) ? colour_transparent : common::read<u16>(palette_ram.data(), palette_addr & 0x3ff);
        pixels[column] = colour;
        palette_indices >>= 4;
    }
    
    return pixels;
}

PPU::TileRow PPU::decode_tile_row_8bpp(u32 tile_base, int tile_number, int y, bool horizontal_flip, bool vertical_flip) {
    TileRow pixels;
    int row = (vertical_flip ? (y ^ 7) : y) % 8;
    u32 tile_addr = tile_base + (tile_number * 64) + (row * 8);
    u64 palette_indices = common::read<u64>(vram.data(), tile_addr);

    for (int x = 0; x < 8; x++) {
        int column = horizontal_flip ? (x ^ 7) : x;
        int palette_index = palette_indices & 0xff;
        u16 colour = 0;

        if (palette_index == 0) {
            colour = colour_transparent;
        } else {
            colour = common::read<u16>(palette_ram.data(), (palette_index * 2) & 0x3ff);
        }

        pixels[column] = colour;
        palette_indices >>= 8;
    }
    
    return pixels;
}

} // namespace gba