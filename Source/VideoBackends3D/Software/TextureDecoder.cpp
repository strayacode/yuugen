#include <algorithm>
#include "Common/Log.h"
#include "Common/Types.h"
#include "VideoCommon/VideoUnit.h"
#include "VideoBackends3D/Software/SoftwareRenderer3D.h"

// given some texture coordinates and some parameters,
// get the colour value of the corresponding texel
Colour SoftwareRenderer3D::decode_texture(int s, int t, TextureAttributes attributes) {
    u32 address = (attributes.parameters & 0xFFFF) * 8;
    int format = (attributes.parameters >> 26) & 0x7;
    int width = 8 << ((attributes.parameters >> 20) & 0x7);
    int height = 8 << ((attributes.parameters >> 23) & 0x7);
    int transformation_mode = (attributes.parameters >> 30) & 0x3;
    bool repeat_s_direction = (attributes.parameters >> 16) & 0x1;
    bool repeat_t_direction = (attributes.parameters >> 17) & 0x1;
    bool flip_s_direction = (attributes.parameters >> 18) & 0x1;
    bool flip_t_direction = (attributes.parameters >> 19) & 0x1;

    s >>= 4;
    t >>= 4;

    if (repeat_s_direction) {
        if (flip_s_direction && (s & width)) {
            s = (width - 1) - (s & (width - 1));
        } else {
            s &= width - 1;
        }
    } else {
        s = std::clamp(s, 0, width);
    }

    if (repeat_t_direction) {
        if (flip_t_direction && (t & height)) {
            t = (height - 1) - (t & (height - 1));
        } else {
            t &= height - 1;
        }
    } else {
        t = std::clamp(t, 0, height);
    }

    u32 offset = t * width + s;

    // if (attributes.parameters & (1 << 29)) {
    //     log_fatal("colour0 of paletted textures is transparent");
    // }

    u32 palette_base = attributes.palette_base * 16;

    switch (static_cast<TextureFormat>(format)) {
    case TextureFormat::None:
        return Colour::from_u16(0x0000);
    case TextureFormat::A3I5: {
        int data = video_unit.vram.read_texture_data<u8>(0x06000000 + address + offset);
        int index = data & 0x1F;
        int alpha = (data >> 5) & 0x7;
        u16 colour = video_unit.vram.read_texture_palette<u16>(0x06000000 + palette_base + index * 2);
        alpha = (alpha * 4) + (alpha / 2);
        colour |= alpha << 15;
        return Colour::from_u16(colour);
    }
    case TextureFormat::Palette4Colour: {
        int index = (video_unit.vram.read_texture_data<u8>(0x06000000 + address + (offset / 4)) >> (2 * (offset & 0x3))) & 0x3;

        return Colour::from_u16(video_unit.vram.read_texture_palette<u16>(0x06000000 + palette_base + index * 2));
    }
    case TextureFormat::Palette16Colour: {
        int index = (video_unit.vram.read_texture_data<u8>(0x06000000 + address + (offset / 2)) >> (4 * (offset & 0x1))) & 0xF;

        return Colour::from_u16(video_unit.vram.read_texture_palette<u16>(0x06000000 + palette_base + index * 2));
    }
    case TextureFormat::Palette256Colour: {
        int index = video_unit.vram.read_texture_data<u8>(0x06000000 + address + offset);

        return Colour::from_u16(video_unit.vram.read_texture_palette<u16>(0x06000000 + palette_base + index * 2));
    }
    case TextureFormat::Compressed: {
        // get the 2 bit texel
        int tile_x = s / 4;
        int tile_y = t / 4;
        int tile_size = width / 4;
        int texel_x = s & 0x3;
        int texel_y = t & 0x3;

        // get the appropriate tile (each tile is 32 bits)
        u32 row_address = address + ((tile_x * tile_size) + tile_y) * 4 + texel_y;

        // get palette index data from slot 1
        u32 slot_offset = row_address & 0x1FFFF;
        u32 data_address = 0x06020000 + (slot_offset / 2);

        // if slot2 address is being used then add 0x10000
        if (row_address >= 0x40000) {
            data_address += 0x10000;
        }

        u16 palette_index_data = video_unit.vram.read_texture_data<u16>(data_address);
        u32 palette_offset = palette_index_data & 0x3FFF;
        u8 mode = palette_index_data >> 14;
        palette_base += palette_offset * 4;

        // now get the 2 bit texel
        u8 row = video_unit.vram.read_texture_data<u8>(0x06000000 + row_address);
        u8 index = row >> (texel_x * 2);

        switch (mode) {
        case 0:
            if (index == 3) {
                return Colour::from_u16(0x0000);
            }

            return Colour::from_u16(video_unit.vram.read_texture_palette<u16>(0x06000000 + palette_base + index * 2));
        case 1:
            if (index == 3) {
                return Colour::from_u16(0x0000);
            }

            if (index == 2) {
                Colour c1 = Colour::from_u16(video_unit.vram.read_texture_palette<u16>(0x06000000 + palette_base));
                Colour c2 = Colour::from_u16(video_unit.vram.read_texture_palette<u16>(0x06000000 + palette_base + 2));

                Colour c3;
                c3.r = c1.r / 2 + c2.r / 2;
                c3.g = c1.g / 2 + c2.g / 2;
                c3.b = c1.b / 2 + c2.b / 2;

                return c3;
            }

            return Colour::from_u16(video_unit.vram.read_texture_palette<u16>(0x06000000 + palette_base + index * 2));
        case 2:
            return Colour::from_u16(video_unit.vram.read_texture_palette<u16>(0x06000000 + palette_base + index * 2));
        case 3:
            if (index == 2 || index == 3) {
                int c1_multiplier = index == 2 ? 5 : 3;
                int c2_multiplier = index == 2 ? 3 : 5;
                Colour c1 = Colour::from_u16(video_unit.vram.read_texture_palette<u16>(0x06000000 + palette_base));
                Colour c2 = Colour::from_u16(video_unit.vram.read_texture_palette<u16>(0x06000000 + palette_base + 2));

                Colour c3;
                c3.r = (c1.r * c1_multiplier + c2.r * c2_multiplier) / 8;
                c3.g = (c1.g * c1_multiplier + c2.g * c2_multiplier) / 8;
                c3.b = (c1.b * c1_multiplier + c2.b * c2_multiplier) / 8;

                return c3;
            }

            return Colour::from_u16(video_unit.vram.read_texture_palette<u16>(0x06000000 + palette_base + index * 2));
        default:
            log_fatal("SoftwareRenderer3D: handle compressed mode %d", mode);
        }
        
        return Colour::from_u16(video_unit.vram.read_texture_data<u16>(0x06000000 + address + offset * 2));
    }
    case TextureFormat::A5I3: {
        int data = video_unit.vram.read_texture_data<u8>(0x06000000 + address + offset);
        int index = data & 0x7;
        int alpha = data >> 3;
        u16 colour = video_unit.vram.read_texture_palette<u16>(0x06000000 + palette_base + index * 2);
        alpha = (alpha * 4) + (alpha / 2);
        colour |= alpha << 15;
        return Colour::from_u16(colour);
    }
    case TextureFormat::DirectColour:
        return Colour::from_u16(video_unit.vram.read_texture_data<u16>(0x06000000 + address + offset * 2));
    default:
        log_fatal("SoftwareRenderer3D: handle texture format %d", format);
    }

    return Colour::from_u16(0x7FFF);
}