#include <core/hw/gpu/engine_2d/gpu_2d.h>
#include <core/hw/gpu/gpu.h>

void GPU2D::RenderObjects(u16 line) {
    // check for 1d / 2d mapping
    if (!(DISPCNT & (1 << 4))) {
        // log_warn("handle 2d obj mapping");
    }

    // assume 1d mapping for now
    // each object in oam consists of 6 bytes
    // with 2 bytes for each attribute
    for (int i = 0; i < 128; i++) {
        u16 attribute[3] = {
            ReadOAM<u16>(i * 8),
            ReadOAM<u16>((i * 8) + 2),
            ReadOAM<u16>((i * 8) + 4),
        };

        u8 shape = (attribute[0] >> 14) & 0x3;
        u8 size = (attribute[1] >> 14) & 0x3;
        u16 y = attribute[0] & 0xFF;
        u16 x = attribute[1] & 0x1FF;
        u8 width;
        u8 height;
        u8 horizontal_flip = (attribute[1] >> 12) & 0x1;
        u8 vertical_flip = (attribute[1] >> 13) & 0x1;
        u8 priority = (attribute[2] >> 10) & 0x3;

        width = dimensions[shape][size][0];
        height = dimensions[shape][size][1];
        
        u16 height_difference = line - y;

        // check if an object should be rendered on the current scanline
        if (height_difference < 0 || height_difference >= height) {
            continue;
        }

        u8 mode = (attribute[0] >> 10) & 0x3;

        // for now we will just ignore semi transparent and window objs
        if (mode != 0) {
            // log_fatal("[GPU2D] handle non-normal object %d", mode);
        }

        if (attribute[0] & (1 << 8)) {
            // log_fatal("[GPU2D] handle rotscal object");
        }

        u32 tile_number = attribute[2] & 0x3FF;
        u32 bound = (DISPCNT & (1 << 4)) ? (32 << ((DISPCNT >> 20) & 0x3)) : 32; 
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
                // make sure that when j has been incremented by 8, we move onto the next 8x8 tile
                // and we know that each 8x8 tile occupies 64 bytes in 8bpp mode
                // we must also increment by a byte for each time j is incremented
                u32 offset = obj_base + (j / 8) * 64 + (j % 8);
                u8 palette_index = gpu->ReadVRAM<u8>(offset);
                
                u16 colour;

                if (DISPCNT & (1 << 31)) {
                    if (engine_id == 1) {
                        colour = palette_index == 0 ? 0x8000 : gpu->ReadExtPaletteOBJA<u16>((palette_number * 0xFF + palette_index) * 2);
                    } else {
                        colour = palette_index == 0 ? 0x8000 : gpu->ReadExtPaletteOBJB<u16>((palette_number * 0xFF + palette_index) * 2);
                    }
                } else {
                    colour = palette_index == 0 ? 0x8000 : ReadPaletteRAM<u16>(0x200 + (palette_index * 2));
                }

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
                // make sure that when j has been incremented by 8, we move onto the next 8x8 tile
                // and we know that each 8x8 tile occupies 32 bytes in 4bpp mode
                // we must also increment by a byte for each time j is incremented by 2 (as each pixel occupies 4 bits)
                u32 offset = obj_base + (j / 8) * 32 + ((j % 8) / 2);
                u8 palette_indices = gpu->ReadVRAM<u8>(offset);
                
                // we will only need 4 bits for the palette index, since we have the palette number already
                // if j is odd, then access the top 4 bits of the byte, otherwise access the lower 4 bits of the byte
                u8 palette_index = (j & 0x1) ? (palette_indices >> 4) : (palette_indices & 0xF);

                // now we have the palette index and number, so we can extract a colour from the palette ram
                u16 colour = palette_index == 0 ? 0x8000 : ReadPaletteRAM<u16>(0x200 + (palette_number * 32) + palette_index * 2);
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