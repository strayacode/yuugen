#include <emulator/core/arm.h>
#include <emulator/common/types.h>
#include <emulator/common/arithmetic.h>

// LSL
u32 ARM::lsl(u32 op2, u8 shift_amount) {
    return (op2 << shift_amount);
}

// LSR
u32 ARM::lsr(u32 op2, u8 shift_amount) {
    return (op2 >> shift_amount);
}