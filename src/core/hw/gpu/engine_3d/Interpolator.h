#pragma once

#include "common/GPUTypes.h"

// an interpolate which does perspective correct and linear interpolation
// for colour and texture coordinates
template <int precision>
class Interpolator {
public:
    // we assume that x is between x0 and x1 and x0 < x1
    u32 interpolate(u32 u0, u32 u1, u32 x, u32 x0, u32 x1, u32 w0, u32 w1) {
        if (use_linear_interpolation(w0, w1)) {
            return interpolate_linear(u0, u1, x, x0, x1);
        }

        u32 t0 = x - x0;
        u32 t1 = x1 - x;
        u32 denom = t0 * w0 + t1 * w1;
        u32 factor = 0;

        if (denom != 0) {
            factor = ((t0 * w0) << precision) / denom;
        }

        return (u0 * ((1 << precision) - factor) + u1 * factor) >> precision;
    }

    // we assume that x is between x0 and x1 and x0 < x1
    u32 interpolate_linear(u32 u0, u32 u1, u32 x, u32 x0, u32 x1) {
        u32 denom = x1 - x0;
        u32 factor = 0;

        if (denom != 0) {
            factor = ((x - x0) << precision) / denom;
        }

        return (u0 * ((1 << precision) - factor) + u1 * factor) >> precision;
    }

    Colour interpolate_colour(Colour c1, Colour c2, u32 x, u32 x0, u32 x1, u32 w0, u32 w1) {
        Colour c3;
        c3.r = interpolate(c1.r, c2.r, x, x0, x1, w0, w1);
        c3.g = interpolate(c1.g, c2.g, x, x0, x1, w0, w1);
        c3.b = interpolate(c1.b, c2.b, x, x0, x1, w0, w1);

        return c3;
    }

    // we assume that x is between x0 and x1 and x0 < x1
    u32 interpolate_colour_component(u32 u0, u32 u1, u32 x, u32 x0, u32 x1, u32 w0, u32 w1) {
        u32 t0 = x - x0;
        u32 t1 = x1 - x;
        u32 denom = t0 * w0 + t1 * w1;

        u32 factor = 0;

        if (denom != 0) {
            factor = ((t0 * w0) << precision) / denom;
        }

        if (u0 < u1) {
            return (u0 + ((u1 - u0) * factor)) >> precision;
        } else {
            return (u1 + ((u0 - u1) * ((1 << precision) - factor))) >> precision;
        }
    }

    bool use_linear_interpolation(u32 w0, u32 w1) {
        if constexpr (precision == 9) {
            return w0 == w1 && (w0 & 0x7E) == 0 && (w1 & 0x7E) == 0;
        } else {
            return w0 == w1 && (w0 & 0x7F) == 0 && (w1 & 0x7F) == 0;
        }
    }
    
private:
};