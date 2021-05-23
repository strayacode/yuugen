#include <core/hw/gpu/gpu_2d.h>
#include <core/hw/gpu/gpu.h>

void GPU2D::RenderText(int bg_index, u16 line) {
    u32 character_base = (((BGCNT[bg_index] >> 2) & 0x3) * 0x4000) + (((DISPCNT >> 24) & 0x7) * 0x10000);
    u32 screen_base = (((BGCNT[bg_index] >> 8) & 0x1F) * 0x800) + (((DISPCNT >> 27) & 0x7) * 0x10000);

    // mod 512 since we can have up to a max screen size of 512x512
    u32 y = (line + BGVOFS[bg_index]) % 512;

    // for each increment of y, we add 64 bytes to the screen base
    screen_base += ((y / 8) % 32) * 64;

    u8 screen_size = (BGCNT[bg_index] >> 14) & 0x3;

    // if (screen_size != 0) {
    //     log_fatal("[GPU2D] Handle non 256x256 background map with size %d", screen_size);
    // }

    // if (DISPCNT & (1 << 30)) {
    //     log_fatal("[GPU2D] Handle extended palettes");
    // }

    if (BGCNT[bg_index] & (1 << 7)) {
        // 256 colours / 1 palette
        for (int tile = 0; tile < 256; tile += 8) {
            // get the addr of the tile in the bg map
            // mod 512 since we can have up to a max screen size of 512x512
            u32 x = (tile + BGHOFS[bg_index]) % 512;
            u32 screen_addr = vram_addr + screen_base + ((x / 8) % 32) * 2;
            u16 tile_info;
            if (engine_id == 1) {
                tile_info = gpu->ReadBGA<u16>(screen_addr);
            } else {
                tile_info = gpu->ReadBGB<u16>(screen_addr);
            }

            // now we need to decode what the tile info means
            u32 tile_number = tile_info & 0x3FF;
            u8 horizontal_flip = (tile_info >> 10) & 0x1;
            u8 vertical_flip = (tile_info >> 11) & 0x1;

            // times by 64 as each tile is 64 bytes long
            u32 character_addr = vram_addr + character_base + (tile_number * 64) + (vertical_flip ? ((7 - y % 8) * 8) : ((y % 8) * 8));

            // now we want to write some specific row of 8 pixels in a tile to the bg layer
            for (int j = 0; j < 8; j++) {
                u32 byte_offset = character_addr + (horizontal_flip ? (7 - j) : j);

                u8 palette_index;
                if (engine_id == 1) {
                    palette_index = gpu->ReadBGA<u8>(byte_offset);
                } else {
                    palette_index = gpu->ReadBGB<u8>(byte_offset);
                }

                u16 colour = palette_index == 0 ? COLOUR_TRANSPARENT : ReadPaletteRAM<u16>(palette_index * 2);
                
                bg_layers[bg_index][(256 * line) + tile + j] = colour;
            }
        }
    } else {
        // 16 colours / 16 palettes
        for (int tile = 0; tile < 256; tile += 8) {
            // mod 512 since we can have up to a max screen size of 512x512
            u32 x = (tile + BGHOFS[bg_index]) % 512;
            u32 screen_addr = vram_addr + screen_base + (((x / 8) % 32) * 2);
            u16 tile_info;
            if (engine_id == 1) {
                tile_info = gpu->ReadBGA<u16>(screen_addr);
            } else {
                tile_info = gpu->ReadBGB<u16>(screen_addr);
            }

            // now we need to decode what the tile info means
            u32 tile_number = tile_info & 0x3FF;
            u8 horizontal_flip = (tile_info >> 10) & 0x1;
            u8 vertical_flip = (tile_info >> 11) & 0x1;
            u8 palette_number = (tile_info >> 12) & 0xF;

            // each tile takes up 32 bytes (4 bytes per tile)
            u32 character_addr = vram_addr + character_base + (tile_number * 32);

            u32 tile_offset = character_addr + (vertical_flip ? ((7 - y % 8) * 4) : ((y % 8) * 4));
            u32 palette_indices;
            if (engine_id == 1) {
                palette_indices = gpu->ReadBGA<u32>(tile_offset);
            } else {
                palette_indices = gpu->ReadBGB<u32>(tile_offset);
            }

            for (int j = 0; j < 8; j++) {
                u16 colour = (palette_indices & 0xF) == 0 ? COLOUR_TRANSPARENT : ReadPaletteRAM<u16>((palette_number * 32) + (palette_indices & 0xF) * 2);
                u16 offset = (256 * line) + tile + (horizontal_flip ? (7 - j) : j);

                bg_layers[bg_index][offset] = colour;
                palette_indices >>= 4;
            }
        }
    }
}