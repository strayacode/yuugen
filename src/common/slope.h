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
// span:
// a horizontal line on 1 scanline that represents a small part of a full line
class Slope {
public:
    void Setup(s32 x0, s32 x1, s32 y0, s32 y1) {
        // lines should always be rendered from top to bottom
        if (y1 < y0) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }

        // store x0 in our line with fractional bits since we are potentially adding a fixed pointer number (bias)
        // to it
        line.x0 = x0 << frac_bits;
        line.y0 = y0;

        // check if the line has a negative gradient
        negative = x1 < x0;

        if (negative) {
            std::swap(x0, x1);
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

    // returns the x coordinate of the start of the span with fractional bits
    s32 FracSpanStart(s32 y) {
        s32 displacement = (y - line.y0) * dx;
        if (negative) {
            return line.x0 - displacement;
        } else {
            return line.x0 + displacement;
        }
    }

    // returns the x coordinate of the end of the span with fractional bits
    s32 FracSpanEnd(s32 y) {
        s32 result = FracSpanStart(y);
        // y major lines only have 1 pixel per scanline
        if (x_major) {
            if (negative) {
                result = result + (~mask - (result & ~mask)) - dx + one;
            } else {
                result = (result & mask) + dx - one;
            }
        }
        
        return result;
    }

    // returns the x coordinate of the start of the span without fractional bits
    s32 SpanStart(s32 y) {
        return FracSpanStart(y) >> frac_bits;
    }

    // returns the x coordinate of the end of the span without fractional bits
    s32 SpanEnd(s32 y) {
        return FracSpanEnd(y) >> frac_bits;
    }
    
private:
    static constexpr u32 frac_bits = 18;
    static constexpr u32 one = 1 << frac_bits;

    // bias applied to x start with x major lines
    static constexpr u32 bias = one >> 1;

    // mask used to remove the lower 9 fractional bits
    static constexpr u32 mask = (0xFFFFFFFF) << (frac_bits / 2);

    Line line;

    bool negative;
    bool x_major;
    s32 dx;
    s32 dy;
};