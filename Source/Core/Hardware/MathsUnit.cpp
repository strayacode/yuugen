#include <limits>
#include "Core/Hardware/MathsUnit.h"

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

void MathsUnit::build_mmio(MMIO& mmio) {
    mmio.register_mmio<u16>(
        0x04000280,
        mmio.direct_read<u16>(&divcnt),
        mmio.complex_write<u16>([this](u32, u16 data) {
            divcnt = data;
            start_division();
        })
    );

    mmio.register_mmio<u32>(
        0x04000290,
        mmio.direct_read<u32>(&div_numer),
        mmio.complex_write<u32>([this](u32, u32 data) {
            div_numer = (div_numer & ~0xFFFFFFFF) | data;
            start_division();
        })
    );

    mmio.register_mmio<u32>(
        0x04000294,
        mmio.complex_read<u32>([this](u32) {
            return div_numer >> 32;
        }),
        mmio.complex_write<u32>([this](u32, u32 data) {
            div_numer = (div_numer & 0xFFFFFFFF) | (static_cast<u64>(data) << 32);
            start_division();
        })
    );

    mmio.register_mmio<u32>(
        0x04000298,
        mmio.direct_read<u32>(&div_denom),
        mmio.complex_write<u32>([this](u32, u32 data) {
            div_denom = (div_denom & ~0xFFFFFFFF) | data;
            start_division();
        })
    );

    mmio.register_mmio<u32>(
        0x0400029C,
        mmio.complex_read<u32>([this](u32) {
            return div_denom >> 32;
        }),
        mmio.complex_write<u32>([this](u32, u32 data) {
            div_denom = (div_denom & 0xFFFFFFFF) | (static_cast<u64>(data) << 32);
            start_division();
        })
    );

    mmio.register_mmio<u32>(
        0x040002a0,
        mmio.direct_read<u32, u64>(&div_result),
        mmio.invalid_write<u32>()
    );

    mmio.register_mmio<u32>(
        0x040002a4,
        mmio.complex_read<u32>([this](u32) {
            return div_result >> 32;
        }),
        mmio.invalid_write<u32>()
    );

    mmio.register_mmio<u32>(
        0x040002a8,
        mmio.direct_read<u32, u64>(&divrem_result),
        mmio.invalid_write<u32>()
    );

    mmio.register_mmio<u32>(
        0x040002ac,
        mmio.complex_read<u32>([this](u32) {
            return divrem_result >> 32;
        }),
        mmio.invalid_write<u32>()
    );

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
            // set div_result to +/-1 with sign opposite of div_numer
            // in 32 bit mode the upper 32 bits of div_result are inverted
            divrem_result = numerator;
            div_result = numerator >= 0 ? 0xFFFFFFFF : 0xFFFFFFFF00000001;
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
    // std::sqrt doesn't seem to be 100% accurate with hardware, so use solution found from
    // https://stackoverflow.com/questions/1100090/looking-for-an-efficient-integer-square-root-algorithm-for-arm-thumb2
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