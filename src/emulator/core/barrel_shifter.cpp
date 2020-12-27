#include <emulator/core/arm.h>
#include <emulator/common/types.h>
#include <emulator/common/arithmetic.h>

// LSL
u32 ARM::lsl(u32 op2, u8 shift_amount) {
    if ((shift_amount != 0) && (shift_amount < 32)) {
        set_condition_flag(C_FLAG, (op2 >> (32 - shift_amount)) & 0x1);
        return (op2 << shift_amount);
    }
    return op2;
    
}

// LSR
u32 ARM::lsr(u32 op2, u8 shift_amount) {
    if ((shift_amount != 0) && (shift_amount < 32)) {
        set_condition_flag(C_FLAG, (op2 >> (shift_amount - 1)) & 0x1);
        return (op2 >> shift_amount);
    }
    return op2;
    
}