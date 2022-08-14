#include <limits>
#include <math.h>
#include "Common/Log.h"
#include "Core/Hardware/maths_unit/maths_unit.h"

void MathsUnit::reset() {
    divcnt = 0;
    div_numer = 0;
    div_denom = 0;
    divrem_result = 0;
    divresult = 0;

    sqrt_param = 0;
    sqrtcnt = 0;
    sqrt_result = 0;
}

void MathsUnit::build_mmio(MMIO& mmio) {
    mmio.register_mmio<u16>(
        0x040002b0,
        mmio.direct_read<u16>(&sqrtcnt),
        mmio.complex_write<u16>([this](u32, u16 data) {
            sqrtcnt = data;
            start_square_root();
        })
    );

    mmio.register_mmio<u32>(
        0x040002b4,
        mmio.direct_read<u32>(&sqrt_result),
        mmio.invalid_write<u32>()
    );

    mmio.register_mmio<u32>(
        0x040002b8,
        mmio.direct_read<u32, u64>(&sqrt_param),
        mmio.complex_write<u32>([this](u32, u32 data) {
            sqrt_param = (sqrt_param & ~0xFFFFFFFF) | data;
            printf("sqrt param now %d\n", sqrt_param);
            start_square_root();
        })
    );

    mmio.register_mmio<u32>(
        0x040002bc,
        mmio.complex_read<u32>([this](u32) {
            return sqrt_param >> 32;
        }),
        mmio.complex_write<u32>([this](u32, u32 data) {
            sqrt_param = (sqrt_param & 0xFFFFFFFF) | (static_cast<u64>(data) << 32);
            printf("sqrt param now %d\n", sqrt_param);
            start_square_root();
        })
    );
}

void MathsUnit::start_division() {
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
            // set divresult to +/-1 with sign opposite of div_numer
            // in 32 bit mode the upper 32 bits of divresult are inverted
            divrem_result = numerator;
            divresult = numerator >= 0 ? 0xFFFFFFFF : 0xFFFFFFFF00000001;
        } else if (div_numer == 0x80000000 && denominator == -1) {
            divresult = (u64)((u32)(std::numeric_limits<s32>::min()));
            divrem_result = 0;
        } else {
            divresult = (u64)((s64)(numerator / denominator));
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
            divresult = (numerator >= 0) ? -1 : 1;
        } else if (div_numer == 0x8000000000000000 && denominator == -1) {
            divresult = (u64)(std::numeric_limits<s64>::min());
            divrem_result = 0;
        } else {
            divresult = (u64)((s64)(numerator / denominator));
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
            divresult = (numerator >= 0) ? -1 : 1;
        } else if (div_numer == 0x8000000000000000 && denominator == -1) {
            divresult = (u64)(std::numeric_limits<s64>::min());
            divrem_result = 0;
        } else {
            divresult = (u64)((s64)(numerator / denominator));
            divrem_result = (u64)((s64)(numerator % denominator));
        }
        break;
    }
    default:
        log_fatal("handle division mode %d", division_mode);
    }
}

void MathsUnit::start_square_root() {
    // TODO: check docs again lter
    u8 square_root_mode = sqrtcnt & 0x1;
    switch (square_root_mode) {
    case 0:
        // 32-bit mode input
        sqrt_result = sqrt(sqrt_param & 0xFFFFFFFF);
        break;
    case 1:
        // 64-bit mode input
        sqrt_result = sqrtl(sqrt_param);
        break;
    default:
        log_fatal("implement support for square root mode %d", square_root_mode);
    }
}