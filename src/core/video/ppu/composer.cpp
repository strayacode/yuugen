#include <array>
#include "common/logger.h"
#include "common/memory.h"
#include "core/video/ppu/ppu.h"

namespace core {

void PPU::compose_scanline(int line) {
    for (int x = 0; x < 256; x++) {
        if (bldcnt.special_effect != SpecialEffect::None) {
            compose_pixel_with_special_effects(x, line);
        } else {
            compose_pixel(x, line);
        }
    }
}

void PPU::compose_pixel(int x, int line) {
    u8 enabled = calculate_enabled_layers(x, line);
    u16 backdrop = common::read<u16>(palette_ram);
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

    plot(x, line, rgb555_to_rgb666(pixel));
}

void PPU::compose_pixel_with_special_effects(int x, int line) {
    // u8 enabled = calculate_enabled_layers(x, line);
    // u16 backdrop = common::read<u16>(palette_ram);
    // std::array<u32, 2> pixels;
    // std::array<int, 2> priorities;

    // pixels.fill(backdrop);
    // priorities.fill(4);
    
    // // find the 2 top-most background pixels
    // for (int i = 3; i >= 0; i--) {
    //     if ((enabled >> i) & 0x1) {
    //         if (bgcnt[i].priority <= priorities[0]) {

    //         }
    //     }

    //     if (((enabled >> i) & 0x1) && bgcnt[i].priority <= priority) {
    //         u16 bg_pixel = bg_layers[i][x];
    //         if (bg_pixel != colour_transparent) {
    //             pixel = bg_pixel;
    //             priority = bgcnt[i].priority;
    //         }
    //     }
    // }

    // TODO: convert the pixels to 18-bit, since that's what the ds
    // uses when blending colours

    logger.error("PPU: handle compose_pixel_with_special_effects");
}

void PPU::blend(u32 top, u32 bottom, SpecialEffect special_effect) {
    switch (special_effect) {
    default:
        logger.error("PPU: handle special effect %d", static_cast<int>(special_effect));
    }
}

} // namespace core