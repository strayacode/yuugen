#include "Common/Memory.h"
#include "VideoCommon/GPU.h"
#include "VideoBackends2D/Software/SoftwareRenderer2D.h"

void SoftwareRenderer2D::render_extended(int bg, int line) {
    u32 data_base = vram_addr + ((bgcnt[bg] >> 8) & 0x1F) * 0x4000;
    u8 screen_size = (bgcnt[bg] >> 14) & 0x3;

    u16 size_x = 0;
    u16 size_y = 0;

    switch (screen_size) {
    case 0:
        size_x = 128;
        size_y = 128;
        break;
    case 1:
        size_x = 256;
        size_y = 256;
        break;
    case 2:
        size_x = 512;
        size_y = 256;
        break;
    case 3:
        size_x = 512;
        size_y = 512;
        break;
    }

    // TODO: handle wraparound for all modes
    // if (bgcnt[bg] & (1 << 13)) {
    //     log_fatal("handle");
    // }

    if (!(bgcnt[bg] & (1 << 7))) {
        // 16 bit bgmap entries
        u32 character_base = vram_addr + (((bgcnt[bg] >> 2) & 0xF) * 0x4000) + (((dispcnt >> 24) & 0x7) * 0x10000);
        u32 screen_base = vram_addr + (((bgcnt[bg] >> 8) & 0x1F) * 0x800) + (((dispcnt >> 27) & 0x7) * 0x10000);
        u16 size = 128 << screen_size;
        for (int pixel = 0; pixel < 256; pixel++) {
            // get rotscal coords
            int coord_x = (internal_x[bg - 2] + bgpa[bg - 2] * pixel) >> 8;
            int coord_y = (internal_y[bg - 2] + bgpc[bg - 2] * pixel) >> 8;

            if (bgcnt[bg] & (1 << 13)) {
                coord_x %= size;

                if (coord_x < 0) {
                    log_fatal("handle");
                }

                coord_y %= size;

                if (coord_y < 0) {
                    log_fatal("handle");
                }
            } else if (coord_x < 0 || coord_x >= size || coord_y < 0 || coord_y >= size) {
                continue;
            }

            // get the tile address
            u32 screen_addr = screen_base + ((coord_y / 8) * (size / 8) + (coord_x / 8)) * 2;
            u16 tile_info = gpu.vram.read_vram<u16>(screen_addr);

            u32 tile_number = tile_info & 0x3FF;
            u8 horizontal_flip = (tile_info >> 10) & 0x1;
            u8 vertical_flip = (tile_info >> 11) & 0x1;

            u32 character_addr = character_base + (tile_number * 64);
            character_addr += (vertical_flip ? ((7 - coord_y % 8) * 8) : ((coord_y % 8) * 8));
            character_addr += (horizontal_flip ? (7 - coord_x % 8) : (coord_x % 8));
            
            u8 palette_index = gpu.vram.read_vram<u8>(character_addr);
            u16 colour = Common::read<u16>(palette_ram, (palette_index * 2) & 0x3FF);
            bg_layers[bg][(256 * line) + pixel] = colour;
        }
    } else if ((bgcnt[bg] & (1 << 7)) && (bgcnt[bg] & (1 << 2))) {
        // direct colour bitmap
        for (int pixel = 0; pixel < 256; pixel++) {
            int coord_x = (internal_x[bg - 2] + bgpa[bg - 2] * pixel) >> 8;
            int coord_y = (internal_y[bg - 2] + bgpc[bg - 2] * pixel) >> 8;

            // don't draw the pixel if the x and y coordinates are not inside the dimensions of the bitmap
            if (coord_x < 0 || coord_x >= size_x || coord_y < 0 || coord_y >= size_y) {
                continue;
            }

            u32 data_addr = data_base + (coord_y * size_x + coord_x) * 2;

            u16 colour = gpu.vram.read_vram<u16>(data_addr);
            bg_layers[bg][(256 * line) + pixel] = colour;
        }
    } else {
        // 256 colour bitmap
        for (int pixel = 0; pixel < 256; pixel++) {
            int coord_x = (internal_x[bg - 2] + bgpa[bg - 2] * pixel) >> 8;
            int coord_y = (internal_y[bg - 2] + bgpc[bg - 2] * pixel) >> 8;

            // don't draw the pixel if the x and y coordinates are not inside the dimensions of the bitmap
            if (coord_x < 0 || coord_x >= size_x || coord_y < 0 || coord_y >= size_y) {
                continue;
            }

            u32 data_addr = data_base + (coord_y * size_x + coord_x);
            
            u8 palette_index = gpu.vram.read_vram<u8>(data_addr);
            u16 colour = Common::read<u16>(palette_ram, (palette_index * 2) & 0x3FF);

            bg_layers[bg][(256 * line) + pixel] = colour;
        }
    }

    // increment the internal registers
    internal_x[bg - 2] += bgpb[bg - 2];
    internal_y[bg - 2] += bgpd[bg - 2];
}