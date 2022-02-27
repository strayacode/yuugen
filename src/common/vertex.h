#pragma once

#include "common/types.h"
#include "common/log.h"

// BIG TODO: when we do conversion here it gets swapped back when we call Convert15To24
// we should sort this out and make it cleaner
class Colour {
public:
    void Print() {
        log_debug("r %d g %d b %d", r, g, b);
    }

    // converts to 8 bit precision and returns as hex
    u32 to_u32() {
        u8 new_b = (b * 255) / 31;
        u8 new_g = (g * 255) / 31;
        u8 new_r = (r * 255) / 31;
        return (new_r << 16) | (new_g << 8) | new_b;
    }

    u16 to_u16() {
        return (r << 10) | (g << 5) | b;
    }

    void from_u16(u16 colour) {
        b = colour & 0x1F;
        g = (colour >> 5) & 0x1F;
        r = (colour >> 10) & 0x1F;
    }

    u8 r;
    u8 g;
    u8 b;
};

class Vertex {
public:
    Vertex() {
        x = 0;
        y = 0;
        z = 0;
        w = 0;
        colour = {0, 0, 0};
    }

    s32 x;
    s32 y;
    s32 z;
    s32 w;
    Colour colour;
};