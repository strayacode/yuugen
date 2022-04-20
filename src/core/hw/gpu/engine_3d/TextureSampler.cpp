#include <algorithm>
#include "common/log.h"
#include "common/types.h"
#include "core/hw/gpu/engine_3d/render_engine.h"
#include "core/hw/gpu/gpu.h"

// given some texture coordinates and some parameters,
// get the colour value of the corresponding texel
Colour RenderEngine::sample_texture(int s, int t, u32 parameters) {
    u32 address = (parameters & 0xFFFF) * 8;
    int format = (parameters >> 26) & 0x7;
    int width = 8 << ((parameters >> 20) & 0x7);
    int height = 8 << ((parameters >> 23) & 0x7);
    int transformation_mode = (parameters >> 30) & 0x3;
    bool repeat_s_direction = (parameters >> 16) & 0x1;
    bool repeat_t_direction = (parameters >> 17) & 0x1;

    u32 offset = t * width + s;

    if (!repeat_s_direction) {
        // clamp in s direction
        s = std::clamp(s, 0, width);
    }

    if (!repeat_t_direction) {
        // clamp in t direction
        t = std::clamp(t, 0, height);
    }

    switch (format) {
    case 7:
        return Colour::from_u16(gpu->texture_data.read<u16>(0x06000000 + address + offset * 2));
    default:
        log_fatal("RenderEngine: handle texture format %d", format);
    }

    return Colour::from_u16(0x7FFF);
}