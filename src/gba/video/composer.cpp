#include "common/memory.h"
#include "gba/video/ppu.h"

namespace gba {

void PPU::compose_scanline(int line) {
    for (int x = 0; x < 240; x++) {
        compose_pixel(x, line);
    }
}

void PPU::compose_pixel(int x, int line) {
    u8 enabled = calculate_enabled_layers(x, line);
    u16 backdrop = common::read<u16>(palette_ram.data());
    u16 pixel = backdrop;
    int priority = 4;

    for (int i = 3; i >= 0; i--) {
        if (((enabled >> i) & 0x1) && bgcnt[i].priority <= priority) {
            u16 bg_pixel = bg_layers[i][x];
            if (bg_pixel != colour_transparent) {
                pixel = bg_pixel;
                priority = bgcnt[i].priority;
            }
        }
    }

    if (dispcnt.enable_obj && obj_buffer[x].colour != colour_transparent) {
        if (obj_buffer[x].priority <= priority) {
            pixel = obj_buffer[x].colour;
        }
    }

    plot(x, line, 0xff000000 | rgb555_to_rgb888(pixel));
}

} // namespace gba