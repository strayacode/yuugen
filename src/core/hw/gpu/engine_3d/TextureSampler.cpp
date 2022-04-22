#include <algorithm>
#include "Common/Log.h"
#include "Common/Types.h"
#include "core/hw/gpu/engine_3d/render_engine.h"
#include "core/hw/gpu/gpu.h"

// given some texture coordinates and some parameters,
// get the colour value of the corresponding texel
Colour RenderEngine::sample_texture(int s, int t, TextureAttributes attributes) {
    u32 address = (attributes.parameters & 0xFFFF) * 8;
    int format = (attributes.parameters >> 26) & 0x7;
    int width = 8 << ((attributes.parameters >> 20) & 0x7);
    int height = 8 << ((attributes.parameters >> 23) & 0x7);
    int transformation_mode = (attributes.parameters >> 30) & 0x3;
    bool repeat_s_direction = (attributes.parameters >> 16) & 0x1;
    bool repeat_t_direction = (attributes.parameters >> 17) & 0x1;

    u32 offset = t * width + s;

    if (!repeat_s_direction) {
        // clamp in s direction
        s = std::clamp(s, 0, width);
    }

    if (!repeat_t_direction) {
        // clamp in t direction
        t = std::clamp(t, 0, height);
    }

    u32 palette_base = attributes.palette_base * 16;

    switch (format) {
    case 0:
        return Colour::from_u16(0x0000);
    case 2: {
        int index = (gpu->texture_data.read<u8>(0x06000000 + address + (offset / 4)) >> (2 * (offset & 0x3))) & 0x3;

        return Colour::from_u16(gpu->texture_palette.read<u16>(0x06000000 + palette_base + index * 2));
    }
    case 3: {
        int index = (gpu->texture_data.read<u8>(0x06000000 + address + (offset / 2)) >> (4 * (offset & 0x1))) & 0xF;

        return Colour::from_u16(gpu->texture_palette.read<u16>(0x06000000 + palette_base + index * 2));
    }
    case 4: {
        int index = gpu->texture_data.read<u8>(0x06000000 + address + offset);

        return Colour::from_u16(gpu->texture_palette.read<u16>(0x06000000 + palette_base + index * 2));
    }
    case 7:
        return Colour::from_u16(gpu->texture_data.read<u16>(0x06000000 + address + offset * 2));
    default:
        log_fatal("RenderEngine: handle texture format %d", format);
    }

    return Colour::from_u16(0x7FFF);
}