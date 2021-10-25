#pragma once

#include <common/types.h>
#include <algorithm>

struct Line {
    s32 x0 = 0;
    s32 x1 = 0;
    s32 y0 = 0;
    s32 y1 = 0;
};

// a class that is used to compute accurate slopes
// according the hardware
// many parts of it is taken from StrikerX3's interpolation algorithm

// terminology:
// x major lines:
// these are ones where dx > dy, meaning the line has a gradient from -1 to 1
// y major lines:
// these are ones where dy > dx, meaning the line has a gradient greater than 1 or less than -1
class Slope {
public:
    void Setup(s32 x0, s32 x1, s32 y0, s32 y1) {
        // lines should always be rendered from top to bottom
        if (y1 < y0) {
            std::swap(x0, x1);
            std::swap(y0, t1);
        }

        // store x0 in our line with fractional bits since we are potentially adding a fixed pointer number (bias)
        // to it
        line.x0 = x0 << frac_bits;
        line.y0 = y0;

        // check if the line has a negative gradient
        negative = x1 < x0;

        if (negative) {
            std::swap(x0, x1)
            line.x0--;
        }

        dx = x1 - x0;
        dy = y1 - y0;

        x_major = dx > dy;

        if (x_major || (dx == dy)) {
            // add / subtract bias to xstart (x0) ahead of time
            if (negative) {
                line.x0 -= bias;
            } else {
                line.x0 += bias;
            }
        }

        // now calculate x displacement per scanline
        if (dy != 0) {
            dx *= one / dy;
        } else {
            dx *= one;
        }
    }
private:
    static constexpr frac_bits = 18;
    static constexpr one = 1 << frac_bits;

    // bias applied to x start with x major lines
    static constexpr bias = one >> 1;

    // mask used to remove the lower 9 fractional bits
    static constexpr mask = (~0) << (frac_bits / 2);

    Line line;

    bool negative;
    bool x_major;
    s32 dx;
    s32 dy;
};