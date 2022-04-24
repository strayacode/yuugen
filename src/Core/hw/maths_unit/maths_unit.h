#pragma once

#include "Common/Types.h"

class MathsUnit {
public:
    void Reset();
    void StartDivision();
    void StartSquareRoot();

    // division is started when DIVCNT, DIV_NUMER or DIV_DENOM is written to
    u16 DIVCNT;

    // 64bit Division Numerator
    u64 DIV_NUMER;
    u64 DIV_DENOM;
    u64 DIVREM_RESULT;
    u64 DIV_RESULT;

    // square is started when SQRTCNT or SQRT_PARAM is written to
    u16 SQRTCNT;
    u64 SQRT_PARAM;
    u32 SQRT_RESULT;
};