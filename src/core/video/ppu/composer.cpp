#include <array>
#include <algorithm>
#include "common/logger.h"
#include "common/memory.h"
#include "core/video/ppu/ppu.h"

namespace core {

void PPU::compose_scanline(int line) {
    for (int x = 0; x < 256; x++) {
        // TODO: check if a semi transparent object can override this logic
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
    u8 enabled = calculate_enabled_layers(x, line);
    u16 backdrop = common::read<u16>(palette_ram);
    std::array<u32, 2> pixels;
    std::array<int, 2> priorities;

    pixels.fill(backdrop);
    priorities.fill(4);
    
    // find the 2 top-most background pixels
    for (int i = 3; i >= 0; i--) {
        u16 bg_pixel = bg_layers[i][x];
        if (((enabled >> i) & 0x1) && bg_pixel != colour_transparent) {
            if (bgcnt[i].priority <= priorities[0]) {
                pixels[1] = pixels[0];
                priorities[1] = priorities[0];
                pixels[0] = bg_pixel;
                priorities[0] = bgcnt[i].priority;
            } else if (bgcnt[i].priority <= priorities[1]) {
                pixels[1] = bg_pixel;
                priorities[1] = bgcnt[i].priority;
            }
        }
    }

    // check if an object pixel can replace one of the background pixels
    // TODO: handle object window later
    if (dispcnt.enable_obj && obj_buffer[x].colour != colour_transparent) {
        if (obj_buffer[x].priority <= priorities[0]) {
            pixels[1] = pixels[0];
            pixels[0] = obj_buffer[x].colour;
        } else if (obj_buffer[x].priority <= priorities[1]) {
            pixels[1] = obj_buffer[x].colour;
        }
    }

    // blending operations use 18-bit colours, so convert to that first
    pixels[0] = rgb555_to_rgb666(pixels[0]);
    pixels[1] = rgb555_to_rgb666(pixels[1]);

    u32 blended = blend(pixels[0], pixels[1], bldcnt.special_effect);
    plot(x, line, blended);
}

u32 PPU::blend(u32 top, u32 bottom, SpecialEffect special_effect) {
    int r1 = top & 0x3f;
    int g1 = (top >> 6) & 0x3f;
    int b1 = (top >> 12) & 0x3f;

    switch (special_effect) {
    case SpecialEffect::None:
        return top;
    case SpecialEffect::AlphaBlending: {
        int eva = std::min<int>(16, bldalpha.eva);
        int evb = std::min<int>(16, bldalpha.evb);
        int r2 = bottom & 0x3f;
        int g2 = (bottom >> 6) & 0x3f;
        int b2 = (bottom >> 12) & 0x3f;

        int r = std::min<u8>(63, (r1 * eva + r2 * evb + 8) / 16);
        int b = std::min<u8>(63, (b1 * eva + b2 * evb + 8) / 16);
        int g = std::min<u8>(63, (g1 * eva + g2 * evb + 8) / 16);
        return (b << 12) | (g << 6) | r;
    }
    case SpecialEffect::BrightnessIncrease: {
        int evy = std::min<int>(16, bldy.evy);
        int r = r1 + ((63 - r1) * evy + 8) / 16;
        int g = g1 + ((63 - g1) * evy + 8) / 16;
        int b = b1 + ((63 - b1) * evy + 8) / 16;
        return (b << 12) | (g << 6) | r;
    }
    case SpecialEffect::BrightnessDecrease: {
        int evy = std::min<int>(16, bldy.evy);
        int r = r1 - (r1 * evy + 7) / 16;
        int g = g1 - (g1 * evy + 7) / 16;
        int b = b1 - (b1 * evy + 7) / 16;
        return (b << 12) | (g << 6) | r;
    }
    }
}

} // namespace core