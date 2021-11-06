#include <core/hw/gpu/engine_2d/gpu_2d.h>
#include <core/hw/gpu/gpu.h>

void GPU2D::RenderObjects(u16 line) {
    // loop through every object
    // to see if they lie within the current line
    for (int i = 0; i < 128; i++) {
        u16 attribute[3];

        // obtain the attributes for the current object
        attribute[0] = ReadOAM<u16>(i * 8);
        attribute[1] = ReadOAM<u16>((i * 8) + 2);
        attribute[2] = ReadOAM<u16>((i * 8) + 4);

        u8 shape = (attribute[0] >> 14) & 0x3;
        u8 size = (attribute[1] >> 14) & 0x3;
        u16 y = attribute[0] & 0xFF;
        u16 x = attribute[1] & 0x1FF;
        u8 width = dimensions[shape][size][0];
        u8 height = dimensions[shape][size][1];
        u8 horizontal_flip = (attribute[1] >> 12) & 0x1;
        u8 vertical_flip = (attribute[1] >> 13) & 0x1;
        u8 priority = (attribute[2] >> 10) & 0x3;
        u16 height_difference = line - y;

        if (height_difference < 0 || height_difference >= height) {
            // then don't render the object and move onto the next one
            continue;
        }

        u8 mode = (attribute[0] >> 10) & 0x3;
        u32 tile_number = attribute[2] & 0x3FF;
        u32 bound = (DISPCNT & (1 << 4)) ? (32 << ((DISPCNT >> 20) & 0x3)): 32; 
        u32 obj_base = obj_addr + (tile_number * bound);
        u8 palette_number = (attribute[2] >> 12) & 0xF;

        if (attribute[0] & (1 << 13)) {
            // 256 colour / 1 palette
            // in 1d mapping
            // each sprite will occupy 64 bytes for each 8x8 tile in it
            // for each 8 that height_difference goes we will move to a new tile
            // we must also add on a multiple of 8 for the specific row in an nx8 tile
            if (vertical_flip) {
                // for vertical flip we must opposite tile row in the sprite, as well as the opposite pixel row in the current tile row
                obj_base += (7 - (height_difference % 8)) * 8 + (((height - height_difference - 1) / 8) * width) * 8;
            } else {
                obj_base += (height_difference % 8) * 8 + ((height_difference / 8) * width) * 8;
            }
            
            // draw each pixel in the current row
            for (int j = 0; j < width; j++) {
                u16 colour = Get8BPPOBJPixel(obj_base, palette_number, j % 8, j / 8);
                u16 layer_offset = horizontal_flip ? x + width - j - 1 : x + j;

                // // only update a specific obj pixel if this one has lower priority and is non transparent too
                if (colour != 0x8000) {
                    if (priority < obj_layer[(256 * line) + layer_offset].priority) {
                        obj_layer[(256 * line) + layer_offset].colour = colour;
                        obj_layer[(256 * line) + layer_offset].priority = priority;
                    }
                }
            }
        } else {
            // 16 colour / 16 palette
            // in 1d mapping
            // each sprite will occupy 32 bytes for each 8x8 tile in it
            // for each 8 that height_difference goes we will move to a new tile
            // we must also add on a multiple of 4 for the specific row in an nx8 tile
            if (vertical_flip) {
                // for vertical flip we must opposite tile row in the sprite, as well as the opposite pixel row in the current tile row
                obj_base += (7 - (height_difference % 8)) * 4 + (((height - height_difference - 1) / 8) * width) * 4;
            } else {
                obj_base += (height_difference % 8) * 4 + ((height_difference / 8) * width) * 4;
            }

            // draw each pixel in the current row
            for (int j = 0; j < width; j++) {
                u16 colour = Get4BPPOBJPixel(obj_base, palette_number, j % 8, j / 8);
                u16 layer_offset = horizontal_flip ? x + width - j - 1 : x + j;

                // only update a specific obj pixel if this one has lower priority and is non transparent too
                if (colour != 0x8000) {
                    if (priority < obj_layer[(256 * line) + layer_offset].priority) {
                        obj_layer[(256 * line) + layer_offset].colour = colour;
                        obj_layer[(256 * line) + layer_offset].priority = priority;
                    }
                }
            }
        }
    }
}