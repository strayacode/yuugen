#include <algorithm>
#include "common/logger.h"
#include "nds/video/gpu/backend/software/software_renderer.h"

namespace nds {

u16 SoftwareRenderer::decode_texture(s16 s, s16 t, Polygon& polygon) {
    const auto& parameters = polygon.texture_attributes.parameters;
    const u32 address = parameters.vram_offset * 8;
    const u32 width = 8 << parameters.width;
    const u32 height = 8 << parameters.height;

    // remove the fractional part
    s >>= 4;
    t >>= 4;

    if (parameters.repeat_in_s_direction) {
        if (parameters.flip_in_s_direction && (s & width)) {
            s = (width - 1) - (s & (width - 1));
        } else {
            s &= width - 1;
        }
    } else {
        s = std::clamp<u32>(s, 0, width);
    }

    if (parameters.repeat_in_t_direction) {
        if (parameters.flip_in_t_direction && (t & height)) {
            t = (height - 1) - (t & (height - 1));
        } else {
            t &= height - 1;
        }
    } else {
        t = std::clamp<u32>(t, 0, height);
    }

    const u32 offset = t * width + s;
    const u32 palette_base = polygon.texture_attributes.palette_base * 16;

    if (parameters.colour0) {
        logger.warn("SoftwareRenderer: colour0 of paletted textures is transparent");
    }

    switch (parameters.texture_format) {
    case Polygon::TextureFormat::Direct: {
        const u32 texel_address = address + offset * 2;
        return texture_data.read<u16>(texel_address);
    }
    default:
        logger.warn("SoftwareRenderer: handle texture format %d", static_cast<int>(parameters.texture_format));
        break;
    }

    return 0x7fff;
}

} // namespace nds