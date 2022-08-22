#pragma once

#include "Common/Types.h"
#include "Core/ARM/MMIO.h"

class MathsUnit {
public:
    void reset();
    void build_mmio(MMIO& mmio);

    void start_division();
    void start_square_root();

    // division is started when divcnt, div_numer or div_denom is written to
    u16 divcnt;

    // 64bit Division Numerator
    u64 div_numer;
    u64 div_denom;
    u64 divrem_result;
    u64 div_result;

    // square is started when sqrtcnt or sqrt_param is written to
    u16 sqrtcnt;
    u64 sqrt_param;
    u32 sqrt_result;
};