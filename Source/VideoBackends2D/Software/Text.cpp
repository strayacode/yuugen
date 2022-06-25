#include "Common/Memory.h"
#include "Common/Log.h"
#include "VideoCommon/VideoUnit.h"
#include "VideoBackends2D/Software/SoftwareRenderer2D.h"

void SoftwareRenderer2D::render_text(int bg, int line) {
    u32 character_base = vram_addr + (((bgcnt[bg] >> 2) & 0xF) * 0x4000) + (((dispcnt >> 24) & 0x7) * 0x10000);
    u32 screen_base = vram_addr + (((bgcnt[bg] >> 8) & 0x1F) * 0x800) + (((dispcnt >> 27) & 0x7) * 0x10000);

    // mod 512 since we can have up to a max screen size of 512x512
    u32 y = (line + bgvofs[bg]) % 512;

    // for each increment of y, we add 64 bytes to the screen base
    screen_base += ((y / 8) % 32) * 64;

    u8 screen_size = (bgcnt[bg] >> 14) & 0x3;

    u8 extended_palette_slot = bg + ((bgcnt[bg] >> 13) & 0x1) * 2;

    // for screen blocks, we know that each screen block occupies 0x800 bytes, and are ordered in a linear fashion
    // basic pseudocode:
    // if y >= 256 and screen height is 512, then move by 0x800 bytes if width is 256, otherwise by 0x800 * 2
    // if x >= 256 and screen width is 512, then move by 0x800

    if (y >= 256 && screen_size & 0x2) {
        screen_base += screen_size & 0x1 ? 0x1000 : 0x800;
    } 

    if (bgcnt[bg] & (1 << 7)) {
        // 256 colours / 1 palette
        for (int tile = 0; tile <= 256; tile += 8) {
            // get the addr of the tile in the bg map
            // mod 512 since we can have up to a max screen size of 512x512
            u32 x = (tile + bghofs[bg]) % 512;

            u32 screen_addr = screen_base + ((x / 8) % 32) * 2;

            if (x >= 256 && screen_size & 0x1) {
                screen_addr += 0x800;
            }

            u16 tile_info = video_unit.vram.read_vram<u16>(screen_addr);

            // now we need to decode what the tile info means
            u32 tile_number = tile_info & 0x3FF;
            u8 horizontal_flip = (tile_info >> 10) & 0x1;
            u8 vertical_flip = (tile_info >> 11) & 0x1;
            u8 palette_number = (tile_info >> 12) & 0xF;

            // times by 64 as each tile is 64 bytes long
            u32 character_addr = character_base + (tile_number * 64) + (vertical_flip ? ((7 - y % 8) * 8) : ((y % 8) * 8));

            // now we want to write some specific row of 8 pixels in a tile to the bg layer
            for (int j = 0; j < 8; j++) {
                u32 byte_offset = character_addr + j;

                u8 palette_index = video_unit.vram.read_vram<u8>(byte_offset);

                u16 colour;

                // check if extended palettes are enabled
                if (dispcnt & (1 << 30)) {
                    if (engine == Engine::A) {
                        colour = palette_index == 0 ? 0x8000 : video_unit.vram.read_ext_palette_bga<u16>(extended_palette_slot * 0x2000 + (palette_number * 0xFF + palette_index) * 2);
                    } else {
                        colour = palette_index == 0 ? 0x8000 : video_unit.vram.read_ext_palette_bgb<u16>(extended_palette_slot * 0x2000 + (palette_number * 0xFF + palette_index) * 2);
                    }
                } else {
                    colour = palette_index == 0 ? 0x8000 : Common::read<u16>(palette_ram, (palette_index * 2) & 0x3FF);
                }

                int offset = tile - (x % 8) + (horizontal_flip ? (7 - j) : j);

                if (offset >= 0 && offset <= 0xFF) {
                    bg_layers[bg][(256 * line) + offset] = colour;
                }
            }
        }
    } else {
        // 16 colours / 16 palettes
        for (int tile = 0; tile <= 256; tile += 8) {
            // mod 512 since we can have up to a max screen size of 512x512
            u32 x = (tile + bghofs[bg]) % 512;

            u32 screen_addr = screen_base + (((x / 8) % 32) * 2);

            if (x >= 256 && screen_size & 0x1) {
                screen_addr += 0x800;
            }

            u16 tile_info = video_unit.vram.read_vram<u16>(screen_addr);

            // now we need to decode what the tile info means
            u32 tile_number = tile_info & 0x3FF;
            u8 horizontal_flip = (tile_info >> 10) & 0x1;
            u8 vertical_flip = (tile_info >> 11) & 0x1;
            u8 palette_number = (tile_info >> 12) & 0xF;

            // each tile takes up 32 bytes (4 bytes per tile)
            u32 character_addr = character_base + (tile_number * 32);

            u32 tile_offset = character_addr + (vertical_flip ? ((7 - y % 8) * 4) : ((y % 8) * 4));
            u32 palette_indices = video_unit.vram.read_vram<u32>(tile_offset);

            for (int j = 0; j < 8; j++) {
                u32 palette_addr = (palette_number * 32) + (palette_indices & 0xF) * 2;
                u16 colour = (palette_indices & 0xF) == 0 ? 0x8000 : Common::read<u16>(palette_ram, palette_addr & 0x3FF);
                int offset = tile - (x % 8) + (horizontal_flip ? (7 - j) : j);

                if (offset >= 0 && offset <= 0xFF) {
                    bg_layers[bg][(256 * line) + offset] = colour;
                }

                palette_indices >>= 4;
            }
        }
    }
}