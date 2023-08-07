#include "common/logger.h"
#include "common/memory.h"
#include "common/bits.h"
#include "core/video/ppu/ppu.h"

namespace core {

void PPU::render_objects(int line) {
    for (int i = 0; i < 128; i++) {
        if ((oam[(i * 8) + 1] & 0x3) == 0x2) {
            continue;
        }

        std::array<u16, 3> attributes;
        std::array<s16, 4> affine_parameters;
        attributes[0] = common::read<u16>(oam, i * 8);
        attributes[1] = common::read<u16>(oam, (i * 8) + 2);
        attributes[2] = common::read<u16>(oam, (i * 8) + 4);

        int y = common::get_field<0, 8>(attributes[0]);
        bool affine = common::get_bit<8>(attributes[0]);
        ObjectMode mode = static_cast<ObjectMode>(common::get_field<10, 2>(attributes[0]));
        bool mosaic = common::get_bit<12>(attributes[0]);
        bool is_8bpp = common::get_bit<13>(attributes[0]);
        u8 shape = common::get_field<14, 2>(attributes[0]);
        int x = common::get_field<0, 9>(attributes[1]);
        bool horizontal_flip = !affine & common::get_bit<12>(attributes[1]);
        bool vertical_flip = !affine & common::get_bit<13>(attributes[1]);
        u8 size = common::get_field<14, 2>(attributes[1]);
        u16 tile_number = common::get_field<0, 10>(attributes[2]);
        u16 priority = common::get_field<10, 2>(attributes[2]);
        u16 palette_number = common::get_field<12, 4>(attributes[2]);
        
        if (x >= 256) {
            x -= 512;
        }

        if (y >= 192) {
            y -= 256;
        }

        int width = obj_dimensions[shape][size][0];
        int height = obj_dimensions[shape][size][1];
        int half_width = width / 2;
        int half_height = height / 2;

        x += half_width;
        y += half_height;

        if (mosaic) {
            logger.error("PPU: handle object mosaic");
        }

        if (affine) {
            bool double_size = common::get_bit<9>(attributes[0]);
            u16 group = common::get_field<9, 5>(attributes[1]) * 32;
            
            affine_parameters[0] = common::read<u16>(oam, group + 0x6);
            affine_parameters[1] = common::read<u16>(oam, group + 0xe);
            affine_parameters[2] = common::read<u16>(oam, group + 0x16);
            affine_parameters[3] = common::read<u16>(oam, group + 0x1e);

            if (double_size) {
                x += half_width;
                y += half_height;
                half_width *= 2;
                half_height *= 2;
            }
        } else {
            // for non-affine sprites, we can still use the general affine formula,
            // but instead use the parameters 0x100, 0, 0 and 0x100
            // these parameters get treated as identity values, meaning they don't
            // have any effect in the multiplication
            affine_parameters[0] = 0x100;
            affine_parameters[1] = 0;
            affine_parameters[2] = 0;
            affine_parameters[3] = 0x100;
        }

        if (mode == ObjectMode::SemiTransparent) {
            logger.error("PPU: handle semi transparent mode");
        }

        if (mode == ObjectMode::ObjectWindow) {
            logger.error("PPU: handle object window mode");
        }

        int local_y = line - y;
        if (local_y < -half_height || local_y >= half_height) {
            continue;
        }

        for (int local_x = -half_width; local_x <= half_width; local_x++) {
            u16 colour = 0;
            int global_x = x + local_x;
            if (global_x < 0 || global_x >= 256) {
                continue;
            }

            int transformed_x = (((affine_parameters[0] * local_x) + (affine_parameters[1] * local_y)) >> 8) + (width / 2);
            int transformed_y = (((affine_parameters[2] * local_x) + (affine_parameters[3] * local_y)) >> 8) + (height / 2);

            // make sure the transformed coordinates are still in bounds
            if (transformed_x < 0 || transformed_y < 0 || transformed_x >= width || transformed_y >= height) {
                continue;
            }

            if (horizontal_flip) {
                transformed_x = width - transformed_x - 1;
            }

            if (vertical_flip) {
                transformed_y = height - transformed_y - 1;
            }

            int inner_tile_x = transformed_x % 8;
            int inner_tile_y = transformed_y % 8;
            int tile_x = transformed_x / 8;
            int tile_y = transformed_y / 8;
            int tile_addr = 0;

            if (mode == ObjectMode::Bitmap) {
                logger.error("PPU: handle bitmap mode");
            } else if (is_8bpp) {
                if (dispcnt.tile_obj_mapping) {
                    tile_addr = (tile_number * (32 << dispcnt.tile_obj_1d_boundary)) + (tile_y * width * 8);
                } else {
                    logger.error("PPU: handle 2d mapping 8bpp");
                }

                tile_addr += tile_x * 64;
                colour = decode_obj_pixel_8bpp(tile_addr, palette_number, inner_tile_x, inner_tile_y);
            } else {
                logger.error("PPU: handle 4bpp mode");
            }

            Object& target_obj = obj_buffer[global_x];
            if (colour != colour_transparent) {
                if (priority < target_obj.priority) {
                    target_obj.colour = colour;
                    target_obj.priority = priority;
                }
            }
        }
    }
}

} // namespace core