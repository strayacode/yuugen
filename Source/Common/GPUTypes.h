#pragma once

#include <algorithm>
#include "Common/Types.h"
#include "Common/Log.h"

class Matrix {
public:
    Matrix() {
        // reset the matrix
        // initialise it as a 4x4 identity matrix
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (i == j) {
                    field[i][j] = 1 << 12;
                } else {
                    field[i][j] = 0;
                }
            }
        }
    }

    s32 field[4][4];
};

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

    void Print() {
        log_debug("r %d g %d b %d", r, g, b);
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
            static_cast<u8>(colour & 0x1F),
            static_cast<u8>((colour >> 5) & 0x1F),
            static_cast<u8>((colour >> 10) & 0x1F)
        };
    }

    u8 r = 0;
    u8 g = 0;
    u8 b = 0;
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

struct TextureAttributes {
    u32 parameters = 0;
    u32 palette_base = 0;
};

class Polygon {
public:
    // returns the index of the next vertex
    int Next(int current) {
        if (current == size - 1) {
            return 0;
        } else {
            return current + 1;
        }
    }

    // returns the index of the previous vertex
    int Prev(int current) {
        if (current == 0) {
            return size - 1;
        } else {
            return current - 1;
        }
    }

    Vertex* vertices = nullptr;
    TextureAttributes texture_attributes;
    u32 polygon_attributes = 0;
    int size = 0;
    bool clockwise = false;
};

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
    void Setup(s32 x0, s32 y0, s32 x1, s32 y1) {
        // lines should always be rendered from top to bottom
        if (y0 > y1) {
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
            line.x0--;
            std::swap(x0, x1);
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

    void Setup(Vertex v0, Vertex v1) {
        Setup(v0.x, v0.y, v1.x, v1.y);
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

    bool Negative() {
        return negative;
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

template <int size>
class MatrixStack {
public:
    void Reset() {
        error = false;
        pointer = 0;
    }

    void Push() {
        if (pointer == size) {
            error = true;
            return;
        }

        stack[pointer++] = current;
    }

    void Store(int offset) {
        if (size == 1) {
            stack[0] = current;
        } else if (offset < size) {
            stack[offset] = current;
        } else {
            error = true;
        }
    }

    void Pop(int offset) {
        pointer -= offset;

        if (pointer < 0) {
            pointer = 0;
            error = true;
        } else if (pointer >= size) {
            pointer = size - 1;
            error = true;
        }

        current = stack[pointer];
    }

    void Restore(int offset) {
        if (size == 1) {
            current = stack[0];
        } else if (offset < size) {
            current = stack[offset];
        } else {
            error = true;
        }
    }

    Matrix stack[size];
    Matrix current;
    bool error;
    int pointer;
};