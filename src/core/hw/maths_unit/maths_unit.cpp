#include <core/hw/maths_unit/maths_unit.h>
#include <math.h>

void MathsUnit::Reset() {
    DIVCNT = 0;
    DIV_NUMER = 0;
    DIV_DENOM = 0;
    DIVREM_RESULT = 0;
    DIV_RESULT = 0;

    SQRT_PARAM = 0;
    SQRTCNT = 0;
    SQRT_RESULT = 0;
}

void MathsUnit::StartDivision() {
    // set the division by 0 error bit only if the full 64 bits of DIV_DENOM is 0 (even in 32 bit mode)
    if (DIV_DENOM == 0) {
        DIVCNT |= (1 << 14);
    }

    u8 division_mode = DIVCNT & 0x3;
    switch (division_mode) {
    case 0:
        // 32-bit / 32-bit
        if (DIV_DENOM == 0) {
            // set DIV_RESULT to +/-1 with sign opposite of DIV_NUMER
            // in 32 bit mode the upper 32 bits of DIV_RESULT are inverted
            DIVREM_RESULT = (s64)DIV_RESULT;
            DIV_RESULT = ((s32)DIV_NUMER >= 0) ? -1 : 1;
        } else if (DIV_NUMER == -(s32)0x80000000 && DIV_DENOM == -1) {
            log_fatal("handle MAX/-1");
        } else {
            DIV_RESULT = (s64)((s32)DIV_NUMER / (s32)DIV_DENOM);
            DIVREM_RESULT = (s64)((s32)DIV_NUMER % (s32)DIV_DENOM);
        }
        break;
    case 1: case 3:
        // 64-bit / 32-bit
        if (DIV_DENOM == 0) {
            DIVREM_RESULT = (s64)DIV_RESULT;
            DIV_RESULT = ((s32)DIV_NUMER >= 0) ? -1 : 1;
        } else if (DIV_NUMER == -(s32)0x80000000 && DIV_DENOM == -1) {
            log_fatal("handle MAX/-1");
        } else {
            DIV_RESULT = DIV_NUMER / (s32)DIV_DENOM;
            DIVREM_RESULT = DIV_NUMER % (s32)DIV_DENOM;
        }
        break;
    default:
        log_fatal("handle division mode %d", division_mode);
    }
}

void MathsUnit::StartSquareRoot() {
    // TODO: check docs again lter
    u8 square_root_mode = SQRTCNT & 0x1;
    switch (square_root_mode) {
    case 0:
        // 32-bit mode input
        SQRT_RESULT = sqrt(SQRT_PARAM & 0xFFFFFFFF);
        break;
    case 1:
        // 64-bit mode input
        SQRT_RESULT = sqrtl(SQRT_PARAM);
        break;
    default:
        log_fatal("implement support for square root mode %d", square_root_mode);
    }
}