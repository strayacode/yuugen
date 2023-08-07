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
        u16 palette_number = common::get_field<12, 4>(attributes[4]);
        int width = obj_dimensions[shape][size][0];
        int height = obj_dimensions[shape][size][1];

        if (x >= 256) {
            logger.error("PPU: handle x out of bounds");
        }

        if (y >= 192) {
            logger.error("PPU: handle x out of bounds");
        }

        if (mosaic) {
            logger.error("PPU: handle object mosaic");
        }

        if (affine) {
            bool double_size = common::get_bit<9>(attributes[0]);
            u16 affine_parameter = common::get_field<9, 5>(attributes[1]);

            if (double_size) {
                logger.error("PPU: handle double size parameter");
            }

            logger.error("PPU: handle object affine rendering");
        }

        if (mode == ObjectMode::SemiTransparent) {
            logger.error("PPU: handle semi transparent mode");
        }

        if (mode == ObjectMode::ObjectWindow) {
            logger.error("PPU: handle object window mode");
        }

        if (horizontal_flip) {
            logger.error("PPU: handle horizontal flip");
        }

        if (vertical_flip) {
            logger.error("PPU: handle vertical flip");
        }

        int local_y = line - y;
        if (local_y < 0 || local_y >= height) {
            continue;
        }

        for (int local_x = 0; local_x < width; local_x++) {
            u16 colour = 0;
            int global_x = x + local_x;
            if (global_x < 0 || global_x >= 256) {
                continue;
            }

            if (mode == ObjectMode::Bitmap) {
                logger.error("PPU: handle bitmap mode");
            } else if (is_8bpp) {
                logger.error("PPU: handle 8bpp mode");
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