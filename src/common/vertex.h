#pragma once

#include <common/types.h>
#include <common/log.h>

class Colour {
public:
    void Print() {
        log_debug("r %d g %d b %d", r, g, b);
    }

    // converts to 8 bit precision and returns as hex
    u32 GetHex() {
        u8 new_b = (b * 255) / 31;
        u8 new_g = (g * 255) / 31;
        u8 new_r = (r * 255) / 31;
        return (new_b << 16) | (new_g << 8) | new_r;
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