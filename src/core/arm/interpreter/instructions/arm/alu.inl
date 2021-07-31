#pragma once

void ARMDataProcessing() {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;

    u8 set_flags = (instruction >> 20) & 0x1;
    u8 opcode = (instruction >> 21) & 0xF;
    u8 immediate = (instruction >> 25) & 0x1;

    u32 op1 = regs.r[rn];
    u32 op2 = 0;

    u8 carry_flag = GetConditionFlag(C_FLAG);

    if (immediate) {
        u32 immediate = instruction & 0xFF;
        u8 shift_amount = ((instruction >> 8) & 0xF) << 1;

        op2 = rotate_right(immediate, shift_amount);
        
        // carry flag is only affected if we have a non-zero shift amount
        if (shift_amount != 0) {
            carry_flag = op2 >> 31;
        }
    } else {
        op2 = ARMGetShiftedRegisterDataProcessing(instruction, carry_flag);
    }

    switch (opcode) {
    case 0x0:
        regs.r[rd] = AND(op1, op2, set_flags);
        break;
    case 0x2:
        regs.r[rd] = SUB(op1, op2, set_flags);
        break;
    case 0x4:
        regs.r[rd] = ADD(op1, op2, set_flags);
        break;
    case 0x9:
        TEQ(op1, op2);
        if (set_flags) {
            SetConditionFlag(C_FLAG, carry_flag);
        }
        break;
    case 0xA:
        CMP(op1, op2);
        break;
    case 0xC:
        regs.r[rd] = ORR(op1, op2, set_flags);
        if (set_flags) {
            SetConditionFlag(C_FLAG, carry_flag);
        }
        break;
    case 0xD:
        regs.r[rd] = MOV(op2, set_flags);
        if (set_flags) {
            SetConditionFlag(C_FLAG, carry_flag);
        }
        break;
    case 0xE:
        regs.r[rd] = BIC(op1, op2, set_flags);
        if (set_flags) {
            SetConditionFlag(C_FLAG, carry_flag);
        }
        break;
    default:
        log_fatal("[ARM] Handle data processing opcode %02x", opcode);
    }

    if (rd == 15) {
        if (set_flags) {
            // store the current spsr in cpsr only if in privileged mode
            if (PrivilegedMode()) {
                u32 current_spsr = GetCurrentSPSR();
                
                SwitchMode(current_spsr & 0x1F);
                regs.cpsr = current_spsr;
            } else {
                log_fatal("[ARM] Loading spsr into cpsr in non-privileged mode is undefined behaviour");
            }

            if (IsARM()) {
                ARMFlushPipeline();
            } else {
                ThumbFlushPipeline();
            }
        } else {
            ARMFlushPipeline();
        }
    } else {
        regs.r[15] += 4;
    }
}

// very useless lol, just for consistency
auto MOV(u32 op2, u8 set_flags) -> u32 {
    if (set_flags) {
        SetConditionFlag(N_FLAG, op2 >> 31);
        SetConditionFlag(Z_FLAG, op2 == 0);
    }

    return op2;
}

void TEQ(u32 op1, u32 op2) {
    u32 result = op1 ^ op2;

    SetConditionFlag(N_FLAG, result >> 31);
    SetConditionFlag(Z_FLAG, result == 0);
}

void CMP(u32 op1, u32 op2) {
    u32 result = op1 - op2;

    // set flags
    SetConditionFlag(N_FLAG, result >> 31);
    SetConditionFlag(Z_FLAG, result == 0);
    SetConditionFlag(C_FLAG, SUB_CARRY(op1, op2));
    SetConditionFlag(V_FLAG, SUB_OVERFLOW(op1, op2, result));
}

auto ADD(u32 op1, u32 op2, u8 set_flags) -> u32 {
    u64 result64 = (u64)op1 + (u64)op2;
    u32 result = op1 + op2;

    if (set_flags) {
        SetConditionFlag(N_FLAG, result >> 31);
        SetConditionFlag(Z_FLAG, result == 0);
        SetConditionFlag(C_FLAG, result64 >> 32);
        SetConditionFlag(V_FLAG, ADD_OVERFLOW(op1, op2, result));
    }

    return result;
}

auto SUB(u32 op1, u32 op2, u8 set_flags) -> u32 {
    u32 result = op1 - op2;
    
    if (set_flags) {
        SetConditionFlag(N_FLAG, result >> 31);
        SetConditionFlag(Z_FLAG, result == 0);
        SetConditionFlag(C_FLAG, SUB_CARRY(op1, op2));
        SetConditionFlag(V_FLAG, SUB_OVERFLOW(op1, op2, result)); 
    }

    return result;
}

auto ORR(u32 op1, u32 op2, u8 set_flags) -> u32 {
    u32 result = op1 | op2;

    if (set_flags) {
        SetConditionFlag(N_FLAG, result >> 31);
        SetConditionFlag(Z_FLAG, result == 0);
    }

    return result;
}

auto BIC(u32 op1, u32 op2, u8 set_flags) -> u32 {
    u32 result = op1 & ~op2;

    if (set_flags) {
        SetConditionFlag(N_FLAG, result >> 31);
        SetConditionFlag(Z_FLAG, result == 0);
    }

    return result;
}

auto AND(u32 op1, u32 op2, u8 set_flags) -> u32 {
    u32 result = op1 & op2;
    if (set_flags) {
        SetConditionFlag(N_FLAG, result >> 31);
        SetConditionFlag(Z_FLAG, result == 0);
    }

    return result;

    regs.r[15] += 4;
}

auto ARMGetShiftedRegisterDataProcessing(u32 instruction, u8& carry_flag) -> u32 {
    u8 rm = instruction & 0xF;
    u8 shift_type = (instruction >> 5) & 0x3;
    u8 shift_amount = 0;

    if (instruction & (1 << 4)) {
        u8 rs = (instruction >> 8) & 0xF;
        shift_amount = regs.r[rs] & 0xFF;
    } else {
        shift_amount = (instruction >> 7) & 0x1F;
    }

    u32 op2 = 0;

    switch (shift_type) {
    case 0x0:
        op2 = LSL(regs.r[rm], shift_amount, carry_flag);
        break;
    case 0x1:
        op2 = LSR(regs.r[rm], shift_amount, carry_flag);
        break;
    case 0x2: {
        op2 = ASR(regs.r[rm], shift_amount, carry_flag);
        break;
    }
    case 0x3:
        op2 = ROR(regs.r[rm], shift_amount, carry_flag);
        break;
    }

    return op2;
}

auto LSL(u32 op1, u8 shift_amount, u8& carry_flag) -> u32 {
    if (shift_amount == 0) {
        // carry flag remains unchanged as well as op1
        return op1;
    }

    u32 result = 0;

    if (shift_amount < 32) {
        result = op1 << shift_amount;
    } else if (shift_amount >= 32) {
        if (shift_amount > 32) {
            carry_flag = 0;
        } else {
            carry_flag = op1 & 0x1;
        }
    }

    return result;
}

auto LSR(u32 op1, u8 shift_amount, u8& carry_flag) -> u32 {
    if (shift_amount == 0) {
        // carry flag remains unchanged as well as op1
        return op1;
    }

    u32 result = 0;

    if (shift_amount < 32) {
        result = op1 >> shift_amount;
    } else if (shift_amount >= 32) {
        if (shift_amount > 32) {
            carry_flag = 0;
        } else {
            carry_flag = op1 >> 31;
        }
    }

    return result;
}

auto ASR(u32 op1, u8 shift_amount, u8& carry_flag) -> u32 {
    if (shift_amount == 0) {
        // carry flag remains unchanged as well as op1
        return op1;
    }

    u32 result = 0;

    u8 msb = op1 >> 31;

    if (shift_amount > 32) {
        result = 0xFFFFFFFF * msb;
        carry_flag = msb;
    } else {
        carry_flag = (op1 >> (shift_amount - 1)) & 0x1;
        
        result = (op1 >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
    }

    return result;
}

auto ROR(u32 op1, u8 shift_amount, u8& carry_flag) -> u32 {
    if (shift_amount == 0) {
        // carry flag remains unchanged as well as op1
        return op1;
    }

    u32 result = rotate_right(op1, shift_amount);

    carry_flag = (op1 >> (shift_amount - 1)) & 0x1;

    return result;
}

void ARMMultiply() {
    log_fatal("handle multiply");
}

void ARMMultiplyLong() {
    log_fatal("handle multiply long");
}

void ARMSingleDataSwap() {
    log_fatal("handle single data swap");
}

void ARMCountLeadingZeroes() {
    log_fatal("handle clz");
}

void ARMSaturatingAddSubtract() {
    log_fatal("handle qadd qsub stuff");
}

void ARMSignedHalfwordMultiply() {
    log_fatal("handle stuff");
}