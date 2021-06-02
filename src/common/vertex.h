#pragma once

#include <common/types.h>

struct Vertex {
    Vertex() {
        x = 0;
        y = 0;
        z = 0;
        w = 1 << 12;
    }

    s32 x;
    s32 y;
    s32 z;
    s32 w;
};