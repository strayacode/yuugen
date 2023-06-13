#include "common/logger.h"
#include "common/memory.h"
#include "core/video/ppu/ppu.h"

namespace core {

void PPU::compose_scanline(int line) {
    for (int x = 0; x < 256; x++) {
        compose_pixel(x, line);
    }
}

void PPU::compose_pixel(int x, int line) {
    if (bldcnt.special_effects) {
        logger.todo("PPU: handle blending in composition");
    }

    u8 enabled = calculate_enabled_layers(x, line);
    u16 pixel = common::read<u16>(palette_ram);
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

    if (dispcnt.enable_obj && obj_colour[x] != colour_transparent) {
        if (obj_priority[x] <= priority) {
            pixel = obj_colour[x];
        }
    }

    render_pixel(x, line, rgb555_to_rgb888(pixel) | 0xff000000);
}

} // namespace core