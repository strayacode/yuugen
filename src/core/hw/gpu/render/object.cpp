#include <core/hw/gpu/gpu_2d.h>
#include <core/hw/gpu/gpu.h>

void GPU2D::RenderObjects(u16 line) {
    // check for 1d / 2d mapping
    if (!(DISPCNT & (1 << 4))) {
        log_fatal("handle 2d obj mapping");
    }

    // assume 1d mapping for now
    // each object in oam consists of 6 bytes
    // with 2 bytes for each attribute

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

        // get the y coordinate of the object
        u16 y = attribute[0] & 0xFF;

        // get the x coordinate of the object
        u16 x = attribute[1] & 0x1FF;

        // now we must determine the width and height of the object
        u8 width;
        u8 height;

        // maybe add a lut for simpler code
        switch (shape) {
        case 0:
            // square
            // TODO: make this a shift left
            width = 8 * pow(2, size);
            height = 8 * pow(2, size);
            break;
        case 1:
            // horizontal
            switch (size) {
            case 0:
                width = 16;
                height = 8;
                break;
            case 1:
                width = 32;
                height = 8;
                break;
            case 2:
                width = 32;
                height = 16;
                break;
            case 3:
                width = 64;
                height = 32;
                break;
            }
            break;
        case 2:
            // vertical
            switch (size) {
            case 0:
                width = 8;
                height = 16;
                break;
            case 1:
                width = 8;
                height = 32;
                break;
            case 2:
                width = 16;
                height = 32;
                break;
            case 3:
                width = 32;
                height = 64;
                break;
            }
            break;
        default:
            log_fatal("[GPU2D] Unknown sprite shape %d", shape);
        }

        u16 height_difference = line - y;

        if (height_difference < 0 || height_difference > height) {
            // then don't render the object and move onto the next one
            continue;
        }

        u8 mode = (attribute[0] >> 10) & 0x3;

        if (mode != 0) {
            log_fatal("[GPU2D] handle non-normal object");
        }

        if (attribute[0] & (1 << 8)) {
            log_fatal("[GPU2D] handle rotscal object");
        }

        u32 tile_number = attribute[2] & 0x3FF;

        u32 bound = (DISPCNT & (1 << 4)) ? (32 << ((DISPCNT >> 20) & 0x3)): 32; 

        u32 obj_base = obj_addr + (tile_number * bound);

        if (attribute[0] & (1 << 13)) {
            // 256 colour / 1 palette
            log_fatal("[GPU2D] handle 256 colour / 1 palette");
        } else {
            // 16 colour / 16 palette
            u8 palette_number = (attribute[2] >> 12) & 0xF;

            // in 1d mapping
            // each sprite will occupy 32 bytes for each 8x8 tile in it
            // for each 8 that height_difference goes we will move to a new tile
            // we must also add on a multiple of 4 for the specific row in an nx8 tile
            obj_base += (height_difference % 8) * 4 + ((height_difference / 8) * width) * 4;

            // draw each pixel in the current row
            for (int j = 0; j < width; j++) {
                // make sure that when j has been incremented by 8, we move onto the nejt 8x8 tile
                // and we know that each 8x8 tile occupies 32 bytes in 4bpp mode
                // we must also increment by a byte for each time j is incremented by 2 (as each pijel occupies 4 bits)
                u8 palette_indices;
                u32 offset = obj_base + (j / 8) * 32 + ((j % 8) / 2);
                if (engine_id == 1) {
                    palette_indices = gpu->ReadOBJA<u8>(offset);
                } else {
                    palette_indices = gpu->ReadOBJB<u8>(offset);
                }

                // we will only need 4 bits for the palette indej, since we have the palette number already
                // if j is odd, then access the top 4 bits of the byte, otherwise access the lower 4 bits of the byte
                u8 palette_index = (j & 0x1) ? (palette_indices >> 4) : (palette_indices & 0xF);

                // now we have the palette index and number, so we can extract a colour from the palette ram
                u16 colour = ReadPaletteRAM<u16>(0x200 + (palette_number * 32) + palette_index * 2);

                // for now we will just write directly to framebuffer
                // TODO: handle obj priority
                framebuffer[(256 * line) + x + j] = Convert15To24(colour);
            }
        }
    }
}