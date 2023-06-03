#pragma once

#include "common/types.h"

namespace core {

class MathsUnit {
public:
    void reset();
    
    u16 read_divcnt() { return divcnt; }
    u64 read_div_numer() { return div_numer; }
    u64 read_div_denom() { return div_denom; }
    u64 read_divrem_result() { return divrem_result; }
    u64 read_div_result() { return div_result; }
    u16 read_sqrtcnt() { return sqrtcnt; }
    u64 read_sqrt_param() { return sqrt_param; }
    u32 read_sqrt_result() { return sqrt_result; }

    void write_divcnt(u16 value, u32 mask);
    void write_div_numer(u64 value, u64 mask);
    void write_div_denom(u64 value, u64 mask);
    void write_sqrtcnt(u16 value, u32 mask);
    void write_sqrt_param(u64 value, u64 mask);
    
private:
    void start_division();
    void start_square_root();

    u16 divcnt;
    u64 div_numer;
    u64 div_denom;
    u64 divrem_result;
    u64 div_result;

    u16 sqrtcnt;
    u64 sqrt_param;
    u32 sqrt_result;
};

} // namespace core