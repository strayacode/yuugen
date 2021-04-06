#pragma once

#include <util/types.h>
#include <util/log.h>

// deals with math related operations such as sqrt and division
struct MathsUnit {
    void Reset();
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

    void StartDivision();
    void StartSquareRoot();
};