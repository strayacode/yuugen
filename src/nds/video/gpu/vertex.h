#pragma once

#include "common/types.h"

namespace nds {

// BIG TODO: when we do conversion here it gets swapped back when we call Convert15To24
// we should sort this out and make it cleaner
class Colour {
public:
    Colour() {
        r = 0;
        g = 0;
        b = 0;
    }

    Colour(u8 r, u8 g, u8 b) {
        this->r = r;
        this->g = g;
        this->b = b;
    }

    // converts to 8 bit precision and returns as hex
    u32 to_u32() {
        u8 new_b = (b * 255) / 31;
        u8 new_g = (g * 255) / 31;
        u8 new_r = (r * 255) / 31;
        return (new_b << 16) | (new_g << 8) | new_r;
    }

    u16 to_u16() {
        return (b << 10) | (g << 5) | r;
    }

    static Colour from_u16(u16 colour) {
        return Colour{
            static_cast<u8>(colour & 0x1f),
            static_cast<u8>((colour >> 5) & 0x1f),
            static_cast<u8>((colour >> 10) & 0x1f)
        };
    }

    u8 r{0};
    u8 g{0};
    u8 b{0};
};

class Vertex {
public:
    Vertex() {
        x = 0;
        y = 0;
        z = 0;
        w = 0;
        colour = {0, 0, 0};
        s = 0;
        t = 0;
    }

    s32 x;
    s32 y;
    s32 z;
    s32 w;
    Colour colour;

    // texture coordinates
    // these are signed 16 bit numbers with a 4 bit fractional
    // part and an 11 bit integer part
    // e.g. 1.0 = 1 << 4
    s16 s;
    s16 t;
};

} // namespace nds