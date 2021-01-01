#include <emulator/core/arm.h>
#include <emulator/common/types.h>
#include <emulator/common/arithmetic.h>

// LSL
u32 ARM::lsl(u32 op2, u8 shift_amount) {
    u32 result;
    if (shift_amount < 32) {
        if (shift_amount != 0) {
            set_condition_flag(C_FLAG, (op2 >> (32 - shift_amount)) & 0x1);
        }
        
        result = (op2 << shift_amount);
    } else {
        result = 0;
        if (shift_amount == 32) {
            set_condition_flag(C_FLAG, op2 & 0x1);
        } else {
            set_condition_flag(C_FLAG, false);
        }
    }
    return result;
}

// LSR
u32 ARM::lsr(u32 op2, u8 shift_amount) {
    if ((shift_amount != 0) && (shift_amount < 32)) {
        set_condition_flag(C_FLAG, (op2 >> (shift_amount - 1)) & 0x1);
        return (op2 >> shift_amount);
    }
    return op2;  
}

// ASR
u32 ARM::asr(u32 op2, u8 shift_amount) {
    // get msb for use when performing asr 
    u8 msb = op2 >> 31;

    if (shift_amount >= 32 || shift_amount == 0) {
        set_condition_flag(C_FLAG, msb);
        // this is because when shifted by 32 or more all the bits will become equal to bit 31
        return 0xFFFFFFFF * msb;
    }

    // if the shift_amount is less than 32:
    set_condition_flag(C_FLAG, (op2 >> (shift_amount - 1) & 0x1));
    // perform asr
    return (op2 >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
}

// ROR
u32 ARM::ror(u32 op2, u8 shift_amount) {
    // check if shift_amount = 0 for rrx
    if (shift_amount == 0) {
        // rrx
        // shifts op2 right by 1 and replaces bit 31 with the carry flag
        u8 lsb = op2 & 0x1;
        u32 result = (op2 >> 1) | (get_condition_flag(C_FLAG) << 31);
        // set c flag to bit 0 of op2
        set_condition_flag(C_FLAG, lsb);
        return result;
    } else {
        // perform a regular ror
        u32 result = (op2 >> shift_amount) | (op2 << (32 - shift_amount));
        // c flag is bit 31 of result as when op2 is rotated the last bit, bit 31 will correspond to the carry out
        set_condition_flag(C_FLAG, result >> 31);
        return result;
    }
}