#pragma once

namespace nds {

#include <algorithm>
#include "common/types.h"
#include "nds/video/gpu/vertex.h"

struct Line {
    s32 x0{0};
    s32 x1{0};
    s32 y0{0};
    s32 y1{0};
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
    void setup(s32 x0, s32 y0, s32 x1, s32 y1) {
        // lines should always be rendered from top to bottom
        if (y0 > y1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }

        // store x0 in our line with fractional bits since we are potentially adding a fixed pointer number (bias)
        // to it
        line.x0 = x0 << FRAC_BITS;
        line.y0 = y0;

        // check if the line has a negative gradient
        negative = x1 < x0;

        if (negative) {
            line.x0--;
            std::swap(x0, x1);
        }

        dx = x1 - x0;
        dy = y1 - y0;

        major = dx > dy;

        if (major || (dx == dy)) {
            // add / subtract bias to xstart (x0) ahead of time
            if (negative) {
                line.x0 -= BIAS;
            } else {
                line.x0 += BIAS;
            }
        }

        // now calculate x displacement per scanline
        if (dy != 0) {
            dx *= ONE / dy;
        } else {
            dx *= ONE;
        }
    }

    void setup(Vertex v0, Vertex v1) {
        setup(v0.x, v0.y, v1.x, v1.y);
    }

    // returns the x coordinate of the start of the span with fractional bits
    s32 frac_span_start(s32 y) {
        s32 displacement = (y - line.y0) * dx;
        if (negative) {
            return line.x0 - displacement;
        } else {
            return line.x0 + displacement;
        }
    }

    // returns the x coordinate of the end of the span with fractional bits
    s32 frac_span_end(s32 y) {
        s32 result = frac_span_start(y);

        // y major lines only have 1 pixel per scanline
        if (major) {
            if (negative) {
                result = result + (~MASK - (result & ~MASK)) - dx + ONE;
            } else {
                result = (result & MASK) + dx - ONE;
            }
        }
        
        return result;
    }

    // returns the x coordinate of the start of the span without fractional bits
    s32 span_start(s32 y) {
        return frac_span_start(y) >> FRAC_BITS;
    }

    // returns the x coordinate of the end of the span without fractional bits
    s32 span_end(s32 y) {
        return frac_span_end(y) >> FRAC_BITS;
    }

    bool is_negative() {
        return negative;
    }
    
private:
    static constexpr u32 FRAC_BITS = 18;
    static constexpr u32 ONE = 1 << FRAC_BITS;

    // bias applied to x start with x major lines
    static constexpr u32 BIAS = ONE >> 1;

    // mask used to remove the lower 9 fractional bits
    static constexpr u32 MASK = (0xFFFFFFFF) << (FRAC_BITS / 2);

    Line line;
    bool negative;
    bool major;
    s32 dx;
    s32 dy;
};

} // namespace nds