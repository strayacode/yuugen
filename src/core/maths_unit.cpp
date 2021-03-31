#include <core/maths_unit.h>

void MathsUnit::Reset() {
    DIVCNT = 0;
    DIV_NUMER = 0;
    DIV_DENOM = 0;

    SQRT_PARAM = 0;
    SQRTCNT = 0;
}