#include <core/hw/gpu/gpu_2d.h>
#include <core/hw/gpu/gpu.h>

void GPU2D::RenderText(int bg_index, u16 line) {
    u32 character_base = (((BGCNT[bg_index] >> 2) & 0x3) * 0x4000) + (((DISPCNT >> 24) & 0x7) * 0x10000);
    u32 screen_base = (((BGCNT[bg_index] >> 8) & 0x1F) * 0x800) + (((DISPCNT >> 27) & 0x7) * 0x10000);

    // add 64 bytes for each line of tiles (64 bytes)
    screen_base += ((line / 8) % 32) * 64;
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
            u32 screen_addr = vram_addr + screen_base + ((tile / 8) % 32) * 2;
            u16 tile_info;
            if (engine_id == 1) {
                tile_info = gpu->ReadBGA(screen_addr);
            } else {
                tile_info = gpu->ReadBGB(screen_addr);
            }

            // now we need to decode what the tile info means
            u32 tile_number = tile_info & 0x3FF;
            u8 horizontal_flip = (tile_info >> 10) & 0x1;
            u8 vertical_flip = (tile_info >> 11) & 0x1;

            // TODO: add horizontal and vertical flip

            // times by 64 as each tile is 64 bytes long
            u32 character_addr = vram_addr + character_base + (tile_number * 64);

            // now we want to write some specific row of 8 pixels in a tile to the bg layer
            for (int j = 0; j < 8; j += 2) {
                u32 byte_offset = character_addr + ((line % 8) * 8) + j;

                u16 palette_indices;
                if (engine_id == 1) {
                    palette_indices = gpu->ReadBGA(byte_offset);
                } else {
                    palette_indices = gpu->ReadBGB(byte_offset);
                }

                // now we have the palette indices for 2 pixels in a row
                u16 colour1 = ReadPaletteRAM(palette_indices & 0xFF);
                u16 colour2 = ReadPaletteRAM(palette_indices >> 8);
                
                layers[bg_index][(256 * line) + tile + j] = Convert15To24(colour1);
                layers[bg_index][(256 * line) + tile + j + 1] = Convert15To24(colour2);
            }
        }
    } else {
        // 16 colours / 16 palettes
        for (int tile = 0; tile < 256; tile += 8) {
            u32 screen_addr = vram_addr + screen_base + (((tile / 8) % 32) * 2);
            u16 tile_info;
            if (engine_id == 1) {
                tile_info = gpu->ReadBGA(screen_addr);
            } else {
                tile_info = gpu->ReadBGB(screen_addr);
            }

            // printf("tile info %04x\n", tile_info);

            // now we need to decode what the tile info means
            u32 tile_number = tile_info & 0x3FF;
            u8 horizontal_flip = (tile_info >> 10) & 0x1;
            u8 vertical_flip = (tile_info >> 11) & 0x1;
            u8 palette_number = (tile_info >> 12) & 0xF;

            // each tile takes up 32 bytes (4 bytes per tile)
            u32 character_addr = vram_addr + character_base + (tile_number * 32);

            u32 tile_offset = character_addr + ((line % 8) * 4);
            // printf("tile offest is %08x\n", tile_offset);
            u32 palette_indices;
            if (engine_id == 1) {
                palette_indices = (gpu->ReadBGA(tile_offset + 2) << 16) | (gpu->ReadBGA(tile_offset));
            } else {
                palette_indices = (gpu->ReadBGB(tile_offset + 2) << 16) | (gpu->ReadBGB(tile_offset));
            }

            for (int j = 0; j < 8; j++) {
                u16 colour = ReadPaletteRAM((palette_number * 32) + (palette_indices & 0xF) * 2);
                layers[bg_index][(256 * line) + tile + j] = Convert15To24(colour);
                palette_indices >>= 4;
            }
        }
    }
}