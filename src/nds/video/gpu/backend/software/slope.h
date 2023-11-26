#pragma once

namespace nds {

#include <algorithm>
#include "common/types.h"
#include "nds/video/gpu/vertex.h"

struct Point {
    s32 x;
    s32 y;
};

// a class that is used to compute accurate slopes
// according the hardware
// many parts of it are taken from StrikerX3's interpolation algorithm

// terminology:
// x major lines:
// these are ones where dx > dy, meaning the line has a gradient from -1 to 1
// these can have more than 2 pixels per scanline
// y major lines:
// these are ones where dy > dx, meaning the line has a gradient greater than 1 or less than -1
// these will always have 1 pixel per scanline
// span:
// a horizontal line on 1 scanline that represents a small part of a full line
class Slope {
public:
    Slope(const Vertex& v0, const Vertex& v1) {
        setup(v0.x, v0.y, v1.x, v1.y);
    }

    void setup(s32 x0, s32 y0, s32 x1, s32 y1) {
        // lines should always be rendered from top to bottom
        if (y0 > y1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }

        // store x0 in our line with fractional bits since we are potentially adding a fixed pointer number (bias)
        // to it
        p0.x = x0 << FRAC_BITS;
        p0.y = y0;

        // check if the line has a negative gradient
        negative = x1 < x0;
        if (negative) {
            p0.x--;
            std::swap(x0, x1);
        }

        s32 dx = x1 - x0;
        s32 dy = y1 - y0;
        x_major = dx > dy;

        if (x_major || (dx == dy)) {
            // add / subtract bias to xstart (x0) ahead of time
            if (negative) {
                p0.x -= BIAS;
            } else {
                p0.x += BIAS;
            }
        }

        this->dx = dx;

        // now calculate x displacement per scanline
        if (dy != 0) {
            this->dx *= ONE / dy;
        } else {
            this->dx *= ONE;
        }
    }

    // returns the x coordinate of the start of the span without fractional bits
    s32 span_start(s32 y) {
        return frac_span_start(y) >> FRAC_BITS;
    }

    // returns the x coordinate of the end of the span without fractional bits
    s32 span_end(s32 y) {
        return frac_span_end(y) >> FRAC_BITS;
    }

    bool is_x_major() const { return x_major; }
    bool is_negative() const { return negative; }
    
private:
    // returns the x coordinate of the start of the span with fractional bits
    s32 frac_span_start(s32 y) {
        s32 displacement = (y - p0.y) * dx;
        if (negative) {
            return p0.x - displacement;
        } else {
            return p0.x + displacement;
        }
    }

    // returns the x coordinate of the end of the span with fractional bits
    s32 frac_span_end(s32 y) {
        s32 result = frac_span_start(y);

        // y major lines only have 1 pixel per scanline
        if (x_major) {
            if (negative) {
                result = result + (~MASK - (result & ~MASK)) - dx + ONE;
            } else {
                result = (result & MASK) + dx - ONE;
            }
        }
        
        return result;
    }

    static constexpr u32 FRAC_BITS = 18;
    static constexpr u32 ONE = 1 << FRAC_BITS;

    // bias applied to x start with x major lines
    static constexpr u32 BIAS = ONE >> 1;

    // mask used to remove the lower 9 fractional bits
    static constexpr u32 MASK = (0xffffffff) << (FRAC_BITS / 2);

    Point p0;
    s32 dx;
    bool negative;
    bool x_major;
};

} // namespace nds