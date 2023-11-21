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
    u32 palette_base = polygon.texture_attributes.palette_base * 16;

    // if (parameters.colour0) {
    //     logger.warn("SoftwareRenderer: colour0 of paletted textures is transparent");
    // }

    switch (parameters.texture_format) {
    case Polygon::TextureFormat::None:
        return 0x0000;
    case Polygon::TextureFormat::A3I5: {
        int data = texture_data.read<u8>(address + offset);
        int index = data & 0x1f;
        int alpha = (data >> 5) & 0x7;
        u16 colour = texture_palette.read<u16>(palette_base + index * 2) & 0x7fff;
        alpha = (alpha * 4) + (alpha / 2);
        colour |= alpha << 15;
        return colour;
    }
    case Polygon::TextureFormat::Colour4: {
        const int index = (texture_data.read<u8>(address + (offset / 4)) >> (2 * (offset & 0x3))) & 0x3;
        return texture_palette.read<u16>((palette_base >> 1) + index * 2) & 0x7fff;
    }
    case Polygon::TextureFormat::Colour16: {
        const int index = (texture_data.read<u8>(address + (offset / 2)) >> (4 * (offset & 0x1))) & 0xf;
        return texture_palette.read<u16>(palette_base + index * 2);
    }
    case Polygon::TextureFormat::Colour256: {
        const int index = texture_data.read<u8>(address + offset);
        return texture_palette.read<u16>(palette_base + index * 2);
    }
    case Polygon::TextureFormat::Compressed: {
        // get the 2 bit texel
        int tile_x = s / 4;
        int tile_y = t / 4;
        int tile_size = width / 4;
        int texel_x = s & 0x3;
        int texel_y = t & 0x3;

        // get the appropriate tile (each tile is 32 bits)
        u32 row_address = address + ((tile_x * tile_size) + tile_y) * 4 + texel_y;

        // get palette index data from slot 1
        u32 slot_offset = row_address & 0x1ffff;
        u32 data_address = 0x20000 + (slot_offset / 2);

        // if slot2 address is being used then add 0x10000
        if (row_address >= 0x40000) {
            data_address += 0x10000;
        }

        u16 palette_index_data = texture_data.read<u16>(data_address);
        u32 palette_offset = palette_index_data & 0x3fff;
        u8 mode = palette_index_data >> 14;
        palette_base += palette_offset * 4;

        // now get the 2 bit texel
        u8 row = texture_data.read<u8>(row_address);
        u8 index = row >> (texel_x * 2);

        switch (mode) {
        case 0:
            if (index == 3) {
                return 0x0000;
            }

            return texture_palette.read<u16>(palette_base + index * 2);
        case 1:
            if (index == 3) {
                return 0x0000;
            }

            if (index == 2) {
                Colour c1 = Colour::from_u16(texture_palette.read<u16>(palette_base));
                Colour c2 = Colour::from_u16(texture_palette.read<u16>(palette_base + 2));

                Colour c3;
                c3.r = c1.r / 2 + c2.r / 2;
                c3.g = c1.g / 2 + c2.g / 2;
                c3.b = c1.b / 2 + c2.b / 2;

                return c3.to_u16();
            }

            return texture_palette.read<u16>(palette_base + index * 2);
        case 2:
            return texture_palette.read<u16>(palette_base + index * 2);
        case 3:
            if (index == 2 || index == 3) {
                int c1_multiplier = index == 2 ? 5 : 3;
                int c2_multiplier = index == 2 ? 3 : 5;
                Colour c1 = Colour::from_u16(texture_palette.read<u16>(palette_base));
                Colour c2 = Colour::from_u16(texture_palette.read<u16>(palette_base + 2));

                Colour c3;
                c3.r = (c1.r * c1_multiplier + c2.r * c2_multiplier) / 8;
                c3.g = (c1.g * c1_multiplier + c2.g * c2_multiplier) / 8;
                c3.b = (c1.b * c1_multiplier + c2.b * c2_multiplier) / 8;

                return c3.to_u16();
            }

            return texture_palette.read<u16>(palette_base + index * 2);
        default:
            logger.todo("GPU: handle compressed mode %d", mode);
        }
        
        return texture_data.read<u16>(address + offset * 2);
    }
    case Polygon::TextureFormat::A5I3: {
        int data = texture_data.read<u8>(address + offset);
        int index = data & 0x7;
        int alpha = data >> 3;
        u16 colour = texture_palette.read<u16>(palette_base + index * 2);
        alpha = (alpha * 4) + (alpha / 2);
        colour |= alpha << 15;
        return colour;
    }
    case Polygon::TextureFormat::Direct: {
        const u32 texel_address = address + offset * 2;
        return texture_data.read<u16>(texel_address);
    }
    }
}

} // namespace nds