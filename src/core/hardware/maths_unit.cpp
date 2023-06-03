#include <limits>
#include "core/hardware/maths_unit.h"

namespace core {

void MathsUnit::reset() {
    divcnt = 0;
    div_numer = 0;
    div_denom = 0;
    divrem_result = 0;
    div_result = 0;

    sqrt_param = 0;
    sqrtcnt = 0;
    sqrt_result = 0;
}

void MathsUnit::write_divcnt(u16 value, u32 mask) {
    divcnt = (divcnt & ~mask) | (value & mask);
    start_division();
}

void MathsUnit::write_div_numer(u64 value, u64 mask) {
    div_numer = (div_numer & ~mask) | (value & mask);
    start_division();
}

void MathsUnit::write_div_denom(u64 value, u64 mask) {
    div_denom = (div_denom & ~mask) | (value & mask);
    start_division();
}

void MathsUnit::write_sqrtcnt(u16 value, u32 mask) {
    sqrtcnt = (sqrtcnt & ~mask) | (value & mask);
    start_square_root();
}

void MathsUnit::write_sqrt_param(u64 value, u64 mask) {
    sqrt_param = (sqrt_param & ~mask) | (value & mask);
    start_square_root();
}

void MathsUnit::start_division() {
    // TODO: make this code less ugly
    // set the division by 0 error bit only if the full 64 bits of div_denom is 0 (even in 32 bit mode)
    if (div_denom == 0) {
        divcnt |= (1 << 14);
    } else {
        divcnt &= ~(1 << 14);
    }

    u8 division_mode = divcnt & 0x3;
    switch (division_mode) {
    case 0: {
        // 32-bit / 32-bit
        s32 numerator = (s32)div_numer;
        s32 denominator = (s32)div_denom;

        if (denominator == 0) {
            // set div_result to +/-1 with sign opposite of div_numer
            // in 32 bit mode the upper 32 bits of div_result are inverted
            divrem_result = numerator;
            div_result = numerator >= 0 ? 0xffffffff : 0xffffffff00000001;
        } else if (div_numer == 0x80000000 && denominator == -1) {
            div_result = (u64)((u32)(std::numeric_limits<s32>::min()));
            divrem_result = 0;
        } else {
            div_result = (u64)((s64)(numerator / denominator));
            divrem_result = (u64)((s64)(numerator % denominator));
        }
        break;
    }
    case 1: case 3: {
        // 64-bit / 32-bit
        s64 numerator = (s64)div_numer;
        s32 denominator = (s32)div_denom;

        if (denominator == 0) {
            divrem_result = numerator;
            div_result = (numerator >= 0) ? -1 : 1;
        } else if (div_numer == 0x8000000000000000 && denominator == -1) {
            div_result = (u64)(std::numeric_limits<s64>::min());
            divrem_result = 0;
        } else {
            div_result = (u64)((s64)(numerator / denominator));
            divrem_result = (u64)((s64)(numerator % denominator));
        }
        break;
    }
    case 2: {
        // 64-bit / 64-bit
        s64 numerator = (s64)div_numer;
        s64 denominator = (s64)div_denom;
        
        if (denominator == 0) {
            divrem_result = numerator;
            div_result = (numerator >= 0) ? -1 : 1;
        } else if (div_numer == 0x8000000000000000 && denominator == -1) {
            div_result = (u64)(std::numeric_limits<s64>::min());
            divrem_result = 0;
        } else {
            div_result = (u64)((s64)(numerator / denominator));
            divrem_result = (u64)((s64)(numerator % denominator));
        }
        break;
    }
    }
}

void MathsUnit::start_square_root() {
    // TODO: make code cleaner
    u64 val = 0;
    u32 res = 0;
    u64 rem = 0;
    u32 prod = 0;
    u32 nbits = 0;
    u32 topshift = 0;

    if (sqrtcnt & 0x1) {
        val = sqrt_param;
        nbits = 32;
        topshift = 62;
    } else {
        val = sqrt_param & 0xFFFFFFFF;
        nbits = 16;
        topshift = 30;
    }

    for (u32 i = 0; i < nbits; i++) {
        rem = (rem << 2) + ((val >> topshift) & 0x3);
        val <<= 2;
        res <<= 1;
        prod = (res << 1) + 1;

        if (rem >= prod) {
            rem -= prod;
            res++;
        }
    }

    sqrt_result = res;
}

} // namespace core