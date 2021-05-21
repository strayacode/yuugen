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
            width = 8 * pow(2, size);
            height = 8 * pow(2, size);
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
    }
}