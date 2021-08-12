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
        u8 rm = instruction & 0xF;
        op2 = regs.r[rm];
        u8 shift_type = (instruction >> 5) & 0x3;
        u8 shift_amount = 0;

        bool immediate = !(instruction & (1 << 4));

        if (immediate) {
            shift_amount = (instruction >> 7) & 0x1F;
        } else {
            u8 rs = (instruction >> 8) & 0xF;
            shift_amount = regs.r[rs] & 0xFF;

            // if either rn or rm is r15 then
            // use as r15 + 12
            if (rn == 15) {
                op1 += 4;
            }

            if (rm == 15) {
                op2 += 4;
            }
        }

        op2 = ARMGetShiftedRegisterDataProcessing(op2, shift_type, shift_amount, carry_flag, immediate);
    }

    switch (opcode) {
    case 0x0:
        regs.r[rd] = AND(op1, op2, set_flags);
        if (set_flags) {
            SetConditionFlag(C_FLAG, carry_flag);
        }
        break;
    case 0x1:
        regs.r[rd] = EOR(op1, op2, set_flags);
        if (set_flags) {
            SetConditionFlag(C_FLAG, carry_flag);
        }
        break;
    case 0x2:
        regs.r[rd] = SUB(op1, op2, set_flags);
        break;
    case 0x4:
        regs.r[rd] = ADD(op1, op2, set_flags);
        break;
    case 0x5:
        regs.r[rd] = ADC(op1, op2, set_flags);
        break;
    case 0x6:
        regs.r[rd] = SBC(op1, op2, set_flags);
        break;
    case 0x7:
        regs.r[rd] = RSC(op1, op2, set_flags);
        break;
    case 0x8:
        TST(op1, op2);
        SetConditionFlag(C_FLAG, carry_flag);
        break;
    case 0x9:
        TEQ(op1, op2);
        SetConditionFlag(C_FLAG, carry_flag);
        break;
    case 0xA:
        CMP(op1, op2);
        break;
    case 0xB:
        CMN(op1, op2);
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
    case 0xF:
        regs.r[rd] = MVN(op2, set_flags);
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

auto MVN(u32 op2, u8 set_flags) -> u32 {
    if (set_flags) {
        SetConditionFlag(N_FLAG, op2 >> 31);
        SetConditionFlag(Z_FLAG, op2 == 0);
    }

    return ~op2;
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

void CMN(u32 op1, u32 op2) {
    u32 result = op1 + op2;

    SetConditionFlag(Z_FLAG, result == 0);
    SetConditionFlag(N_FLAG, result >> 31);
    SetConditionFlag(C_FLAG, ADD_CARRY(op1, op2));
    SetConditionFlag(V_FLAG, ADD_OVERFLOW(op1, op2, result));
}

void TST(u32 op1, u32 op2) {
    u32 result = op1 & op2;

    SetConditionFlag(Z_FLAG, result == 0);
    SetConditionFlag(N_FLAG, result >> 31);
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

auto ADC(u32 op1, u32 op2, u8 set_flags) -> u32 {
    u64 result64 = (u64)op1 + (u64)op2 + (u64)GetConditionFlag(C_FLAG);
    u32 result = (u32)result64;

    if (set_flags) {
        SetConditionFlag(C_FLAG, result64 >> 32);
        SetConditionFlag(Z_FLAG, result == 0);
        SetConditionFlag(N_FLAG, result >> 31);
        SetConditionFlag(V_FLAG, (~(op1 ^ op2) & (op2 ^ result)) >> 31);
    }

    return result;
}

auto SBC(u32 op1, u32 op2, u8 set_flags) -> u32 {
    u32 result = op1 - op2 - !GetConditionFlag(C_FLAG);

    if (set_flags) {
        SetConditionFlag(N_FLAG, result >> 31);
        SetConditionFlag(Z_FLAG, result == 0);
        SetConditionFlag(V_FLAG, SUB_OVERFLOW(op1, op2, result));
        SetConditionFlag(C_FLAG, SUB_CARRY(op1, op2) & SUB_CARRY(op1 - op2, !GetConditionFlag(C_FLAG)));
    }

    return result;
}

auto RSC(u32 op1, u32 op2, u8 set_flags) -> u32 {
    u32 result = op2 - op1 - !GetConditionFlag(C_FLAG);
    
    if (set_flags) {
        SetConditionFlag(N_FLAG, result >> 31);
        SetConditionFlag(Z_FLAG, result == 0);
        SetConditionFlag(C_FLAG, SUB_CARRY(op2, op1));
        SetConditionFlag(V_FLAG, SUB_OVERFLOW(op2, op1, result));
    }

    return result;
}

auto EOR(u32 op1, u32 op2, u8 set_flags) -> u32 {
    u32 result = op1 ^ op2;
    
    if (set_flags) {
        SetConditionFlag(N_FLAG, result >> 31);
        SetConditionFlag(Z_FLAG, result == 0);
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
}

auto ARMGetShiftedRegisterDataProcessing(u32 op2, u8 shift_type, u8 shift_amount, u8& carry_flag, bool immediate) -> u32 {
    switch (shift_type) {
    case 0x0:
        op2 = LSL(op2, shift_amount, carry_flag);
        break;
    case 0x1:
        op2 = LSR(op2, shift_amount, carry_flag, immediate);
        break;
    case 0x2:
        op2 = ASR(op2, shift_amount, carry_flag, immediate);
        break;
    case 0x3:
        op2 = ROR(op2, shift_amount, carry_flag, immediate);
        break;
    }

    return op2;
}

auto LSL(u32 op1, u8 shift_amount, u8& carry_flag) -> u32 {
    if (shift_amount == 0) {
        // carry flag remains unchanged
        // and result is just rm
        return op1;
    }

    u32 result = 0;

    if (shift_amount >= 32) {
        result = 0;

        if (shift_amount > 32) {
            carry_flag = 0;
        } else {
            carry_flag = op1 & 0x1;
        }
    } else {
        result = op1 << shift_amount;
        carry_flag = (op1 >> (32 - shift_amount)) & 0x1;
    }

    return result;
}

auto LSR(u32 op1, u8 shift_amount, u8& carry_flag, bool immediate) -> u32 {
    u32 result = 0;

    if (immediate) {
        if (shift_amount == 0) {
            result = 0;
            carry_flag = op1 >> 31;
        } else {
            result = op1 >> shift_amount;
            carry_flag = (op1 >> (shift_amount - 1)) & 0x1;
        }
    } else {
        if (shift_amount == 0) {
            result = op1;
        } else if (shift_amount < 32) {
            result = op1 >> shift_amount;
            carry_flag = (op1 >> (shift_amount - 1)) & 0x1;
        } else if (shift_amount == 32) {
            result = 0;
            carry_flag = op1 >> 31;
        } else {
            result = 0;
            carry_flag = 0;
        }
    }

    return result;
}

auto ASR(u32 op1, u8 shift_amount, u8& carry_flag, bool immediate) -> u32 {
    u32 result = 0;

    u8 msb = op1 >> 31;

    if (immediate) {
        if (shift_amount == 0) {
            result = 0xFFFFFFFF * msb;
            carry_flag = msb;
        } else {
            result = (op1 >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
            carry_flag = (op1 >> (shift_amount - 1)) & 0x1;
        }
    } else {
        if (shift_amount == 0) {
            result = op1;
        } else if (shift_amount < 32) {
            result = (op1 >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
            carry_flag = (op1 >> (shift_amount - 1)) & 0x1;
        } else {
            result = 0xFFFFFFFF * msb;
            carry_flag = msb;
        }
    }

    return result;
}

auto ROR(u32 op1, u8 shift_amount, u8& carry_flag, bool immediate) -> u32 {
    u32 result = 0;

    if (immediate) {
        if (shift_amount == 0) {
            result = (carry_flag << 31) | (op1 >> 1);
            carry_flag = op1 & 0x1;
        } else {
            result = rotate_right(op1, shift_amount);
            carry_flag = (op1 >> (shift_amount - 1)) & 0x1;
        }
    } else {
        if (shift_amount == 0) {
            result = op1;
        } else if ((shift_amount & 0x1F) == 0) {
            result = op1;
            carry_flag = op1 >> 31;
        } else {
            result = rotate_right(op1, shift_amount & 0x1F);
            carry_flag = (op1 >> ((shift_amount & 0x1F) - 1)) & 0x1;
        }
    }

    return result;
}

template <bool accumulate, bool set_flags>
void ARMMultiply() {
    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rn = (instruction >> 12) & 0xF;
    u8 rd = (instruction >> 16) & 0xF;

    regs.r[rd] = regs.r[rm] * regs.r[rs];

    if constexpr (accumulate) {
        regs.r[rd] += regs.r[rn];
    }

    if (set_flags) {
        SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
        SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
    }

    regs.r[15] += 4;
}

template <bool accumulate, bool set_flags, bool sign>
void ARMMultiplyLong() {
    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rdlo = (instruction >> 12) & 0xF;
    u8 rdhi = (instruction >> 16) & 0xF;

    s64 result = 0;

    if constexpr (sign) {
        result = (s64)(s32)(regs.r[rm]) * (s64)(s32)(regs.r[rs]);
    } else {
        u64 temp_result = (u64)regs.r[rm] * (u64)regs.r[rs];
        result = (s64)temp_result;
    }

    if constexpr (accumulate) {
        s64 temp_result = ((u64)regs.r[rdhi] << 32) | ((u64)regs.r[rdlo]);
        result += temp_result;
    }

    if constexpr (set_flags) {
        SetConditionFlag(N_FLAG, result >> 63);
        SetConditionFlag(Z_FLAG, result == 0);
    }

    regs.r[rdhi] = result >> 32;
    regs.r[rdlo] = result & 0xFFFFFFFF;

    regs.r[15] += 4;
}

void ARMSingleDataSwap() {
    u8 rm = instruction & 0xF;
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;

    u8 byte = (instruction >> 22) & 0x1;
    
    u32 address = regs.r[rn];

    u32 data = 0;

    if (byte) {
        data = ReadByte(address);
    } else {
        data = ReadWord(address);
    }

    if (address & 0x3) {
        int shift_amount = (address & 0x3) * 8;
        data = rotate_right(data, shift_amount);
    }

    if (byte) {
        WriteByte(address, regs.r[rm]);
    } else {
        WriteWord(address, regs.r[rm]);
    }

    regs.r[rd] = data;
    regs.r[15] += 4;
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