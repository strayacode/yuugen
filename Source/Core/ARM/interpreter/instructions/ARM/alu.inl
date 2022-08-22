#pragma once

void arm_data_processing() {
    const bool set_flags = instruction & (1 << 20);
    const bool immediate = (instruction >> 25) & 0x1;
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    u8 opcode = (instruction >> 21) & 0xF;
    
    u32 op1 = regs.r[rn];
    u32 op2 = 0;

    u8 carry_flag = GetConditionFlag(C_FLAG);

    if (immediate) {
        u32 value = instruction & 0xFF;
        u8 shift_amount = ((instruction >> 8) & 0xF) << 1;

        op2 = rotate_right(value, shift_amount);
        
        // carry flag is only affected if we have a non-zero shift amount
        if (shift_amount != 0) {
            carry_flag = op2 >> 31;
        }
    } else {
        u8 rm = instruction & 0xF;
        op2 = regs.r[rm];
        u8 shift_type = (instruction >> 5) & 0x3;
        u8 shift_amount = 0;

        bool shift_imm = !(instruction & (1 << 4));

        if (shift_imm) {
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

        op2 = ARMGetShiftedRegisterDataProcessing(op2, shift_type, shift_amount, carry_flag, shift_imm);
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
    case 0x3:
        regs.r[rd] = SUB(op2, op1, set_flags);
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
        regs.r[rd] = SBC(op2, op1, set_flags);
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

u32 MOV(u32 op2, u8 set_flags) {
    if (set_flags) {
        SetConditionFlag(N_FLAG, op2 >> 31);
        SetConditionFlag(Z_FLAG, op2 == 0);
    }

    return op2;
}

u32 MVN(u32 op2, u8 set_flags) {
    u32 result = ~op2;

    if (set_flags) {
        SetConditionFlag(N_FLAG, result >> 31);
        SetConditionFlag(Z_FLAG, result == 0);
    }

    return result;
}

void TEQ(u32 op1, u32 op2) {
    u32 result = op1 ^ op2;

    SetConditionFlag(N_FLAG, result >> 31);
    SetConditionFlag(Z_FLAG, result == 0);
}

void CMP(u32 op1, u32 op2) {
    u32 result = op1 - op2;

    SetConditionFlag(N_FLAG, result >> 31);
    SetConditionFlag(Z_FLAG, result == 0);
    SetConditionFlag(C_FLAG, op1 >= op2);
    SetConditionFlag(V_FLAG, ((op1 ^ op2) & (op1 ^ result)) >> 31);
}

void CMN(u32 op1, u32 op2) {
    u32 result = op1 + op2;

    SetConditionFlag(Z_FLAG, result == 0);
    SetConditionFlag(N_FLAG, result >> 31);
    SetConditionFlag(C_FLAG, result < op1);
    SetConditionFlag(V_FLAG, (~(op1 ^ op2) & (op2 ^ result)) >> 31);
}

void TST(u32 op1, u32 op2) {
    u32 result = op1 & op2;

    SetConditionFlag(Z_FLAG, result == 0);
    SetConditionFlag(N_FLAG, result >> 31);
}

u32 ADD(u32 op1, u32 op2, u8 set_flags) {
    u32 result = op1 + op2;

    if (set_flags) {
        SetConditionFlag(Z_FLAG, result == 0);
        SetConditionFlag(N_FLAG, result >> 31);
        SetConditionFlag(C_FLAG, result < op1);
        SetConditionFlag(V_FLAG, (~(op1 ^ op2) & (op2 ^ result)) >> 31);
    }

    return result;
}

u32 ADC(u32 op1, u32 op2, u8 set_flags) {
    u64 result64 = (u64)op1 + (u64)op2 + (u64)GetConditionFlag(C_FLAG);
    u32 result = (u32)result64;

    if (set_flags) {
        SetConditionFlag(Z_FLAG, result == 0);
        SetConditionFlag(N_FLAG, result >> 31);
        SetConditionFlag(C_FLAG, result64 >> 32);
        SetConditionFlag(V_FLAG, (~(op1 ^ op2) & (op2 ^ result)) >> 31);
    }

    return result;
}

u32 SBC(u32 op1, u32 op2, u8 set_flags) {
    u32 op3 = GetConditionFlag(C_FLAG) ^ 1;
    u32 result = op1 - op2 - op3;

    if (set_flags) {
        SetConditionFlag(N_FLAG, result >> 31);
        SetConditionFlag(Z_FLAG, result == 0);
        SetConditionFlag(V_FLAG, ((op1 ^ op2) & (op1 ^ result)) >> 31);
        SetConditionFlag(C_FLAG, (u64)op1 >= (u64)op2 + (u64)op3);
    }

    return result;
}

u32 EOR(u32 op1, u32 op2, u8 set_flags) {
    u32 result = op1 ^ op2;
    
    if (set_flags) {
        SetConditionFlag(N_FLAG, result >> 31);
        SetConditionFlag(Z_FLAG, result == 0);
    }

    return result;
}

u32 SUB(u32 op1, u32 op2, u8 set_flags) {
    u32 result = op1 - op2;
    
    if (set_flags) {
        SetConditionFlag(N_FLAG, result >> 31);
        SetConditionFlag(Z_FLAG, result == 0);
        SetConditionFlag(C_FLAG, op1 >= op2);
        SetConditionFlag(V_FLAG, ((op1 ^ op2) & (op1 ^ result)) >> 31);
    }

    return result;
}

u32 ORR(u32 op1, u32 op2, u8 set_flags) {
    u32 result = op1 | op2;

    if (set_flags) {
        SetConditionFlag(N_FLAG, result >> 31);
        SetConditionFlag(Z_FLAG, result == 0);
    }

    return result;
}

u32 BIC(u32 op1, u32 op2, u8 set_flags) {
    u32 result = op1 & ~op2;

    if (set_flags) {
        SetConditionFlag(N_FLAG, result >> 31);
        SetConditionFlag(Z_FLAG, result == 0);
    }

    return result;
}

u32 AND(u32 op1, u32 op2, u8 set_flags) {
    u32 result = op1 & op2;

    if (set_flags) {
        SetConditionFlag(N_FLAG, result >> 31);
        SetConditionFlag(Z_FLAG, result == 0);
    }

    return result;
}

u32 ARMGetShiftedRegisterDataProcessing(u32 op2, u8 shift_type, u8 shift_amount, u8& carry_flag, bool shift_imm) {
    switch (shift_type) {
    case 0x0:
        op2 = LSL(op2, shift_amount, carry_flag);
        break;
    case 0x1:
        op2 = LSR(op2, shift_amount, carry_flag, shift_imm);
        break;
    case 0x2:
        op2 = ASR(op2, shift_amount, carry_flag, shift_imm);
        break;
    case 0x3:
        op2 = ROR(op2, shift_amount, carry_flag, shift_imm);
        break;
    }

    return op2;
}

u32 LSL(u32 op1, u8 shift_amount, u8& carry_flag) {
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

u32 LSR(u32 op1, u8 shift_amount, u8& carry_flag, bool shift_imm) {
    u32 result = 0;

    if (shift_imm) {
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

u32 ASR(u32 op1, u8 shift_amount, u8& carry_flag, bool shift_imm) {
    u32 result = 0;
    u8 msb = op1 >> 31;

    if (shift_imm) {
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

u32 ROR(u32 op1, u8 shift_amount, u8& carry_flag, bool shift_imm) {
    u32 result = 0;

    if (shift_imm) {
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

void arm_multiply() {
    const bool set_flags = (instruction >> 20) & 0x1;
    const bool accumulate = (instruction >> 21) & 0x1;
    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rn = (instruction >> 12) & 0xF;
    u8 rd = (instruction >> 16) & 0xF;
    u32 result = regs.r[rm] * regs.r[rs];

    if (accumulate) {
        result += regs.r[rn];
    }

    regs.r[rd] = result;

    if (set_flags) {
        SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
        SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
    }

    regs.r[15] += 4;
}

void arm_multiply_long() {
    const bool set_flags = (instruction >> 20) & 0x1;
    const bool accumulate = (instruction >> 21) & 0x1;
    const bool sign = (instruction >> 22) & 0x1;
    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rdlo = (instruction >> 12) & 0xF;
    u8 rdhi = (instruction >> 16) & 0xF;

    s64 result = 0;

    if (sign) {
        result = (s64)(s32)(regs.r[rm]) * (s64)(s32)(regs.r[rs]);
    } else {
        u64 temp_result = (u64)regs.r[rm] * (u64)regs.r[rs];
        result = (s64)temp_result;
    }

    if (accumulate) {
        s64 temp_result = ((u64)regs.r[rdhi] << 32) | ((u64)regs.r[rdlo]);
        result += temp_result;
    }

    if (set_flags) {
        SetConditionFlag(N_FLAG, result >> 63);
        SetConditionFlag(Z_FLAG, result == 0);
    }

    regs.r[rdhi] = result >> 32;
    regs.r[rdlo] = result & 0xFFFFFFFF;

    regs.r[15] += 4;
}

void arm_single_data_swap() {
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

void arm_count_leading_zeroes() {
    if (arch == Arch::ARMv4) {
        return;
    }

    u8 rm = instruction & 0xF;
    u8 rd = (instruction >> 12) & 0xF;
    u32 data = regs.r[rm];
    u32 count = 0;

    while (data != 0) {
        data >>= 1;
        count++;
    }

    regs.r[rd] = 32 - count;
    regs.r[15] += 4;
}

void arm_saturating_add_subtract() {
    if (arch == Arch::ARMv4) {
        return;
    }

    u8 rm = instruction & 0xF;
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    u8 opcode = (instruction >> 20) & 0xF;

    u32 result = 0;

    switch (opcode) {
    case 0x0:
        result = regs.r[rm] + regs.r[rn];
        if ((~(regs.r[rm] ^ regs.r[rn]) & (regs.r[rn] ^ result)) >> 31) {
            // set q flag
            SetConditionFlag(Q_FLAG, true);

            // saturate the result
            // this approach avoids an if else statement
            result = 0x80000000 - (result >> 31);
        }
        break;
    case 0x2:
        result = regs.r[rm] - regs.r[rn];
        if (((regs.r[rm] ^ regs.r[rn]) & (regs.r[rm] ^ result)) >> 31) {
            // set q flag
            SetConditionFlag(Q_FLAG, true);

            // since a signed overflow occured with saturated arithmetic, we set the result to the max value
            // according to if its the max negative value (-2^31, 0x80000000) or positive value (2^31 - 1, 0x7FFFFFFF)
            // if greater than the largest positive value (2^31 - 1)
            // saturate the result
            // this approach avoids an if else statement
            result = 0x80000000 - (result >> 31);
        }

        break;
    case 0x4: {
        result = regs.r[rn] * 2;

        if ((regs.r[rn] ^ result) >> 31) {
            // if the last bit has changed then we know a signed overflow occured
            SetConditionFlag(Q_FLAG, true);

            // saturate the result
            // this approach avoids an if else statement
            result = 0x80000000 - (result >> 31);
        }

        u32 old_result = result;
        result += regs.r[rm];
        if ((~(old_result ^ regs.r[rm]) & (regs.r[rm] ^ result)) >> 31) {
            // set q flag
            SetConditionFlag(Q_FLAG, true);

            // since a signed overflow occured with saturated arithmetic, we set the result to the max value
            // according to if its the max negative value (-2^31, 0x80000000) or positive value (2^31 - 1, 0x7FFFFFFF)
            // if greater than the largest positive value (2^31 - 1)
            // saturate the result
            // this approach avoids an if else statement
            result = 0x80000000 - (result >> 31);
        }

        break;
    }
    case 0x6: {
        result = regs.r[rn] * 2;

        if ((regs.r[rn] ^ result) >> 31) {
            // set q flag
            SetConditionFlag(Q_FLAG, true);

            // since a signed overflow occured with saturated arithmetic, we set the result to the max value
            // according to if its the max negative value (-2^31, 0x80000000) or positive value (2^31 - 1, 0x7FFFFFFF)
            // if greater than the largest positive value (2^31 - 1)
            // saturate the result
            // this approach avoids an if else statement
            result = 0x80000000 - (result >> 31);
        }

        u32 old_result = result;
        // now subtract rm
        result = regs.r[rm] - result;
        if (((regs.r[rm] ^ old_result) & (regs.r[rm] ^ result)) >> 31) {
            // set q flag
            SetConditionFlag(Q_FLAG, true);

            // since a signed overflow occured with saturated arithmetic, we set the result to the max value
            // according to if its the max negative value (-2^31, 0x80000000) or positive value (2^31 - 1, 0x7FFFFFFF)
            // if greater than the largest positive value (2^31 - 1)
            // saturate the result
            // this approach avoids an if else statement
            result = 0x80000000 - (result >> 31);
        }

        break;
    }
    default:
        log_fatal("handle opcode %d", opcode);
    }
    
    if (rd == 15) {
        log_fatal("handle");
    }

    regs.r[rd] = result;
    regs.r[15] += 4;
}

void arm_signed_halfword_multiply() {
    if (arch == Arch::ARMv4) {
        return;
    }

    if (((instruction >> 21) & 0xF) == 0xA) {
        log_fatal("handle");
    }

    bool accumulate = ((instruction >> 21) & 0x3) == 0;
    u8 op1 = instruction & 0xF;
    u8 op2 = (instruction >> 8) & 0xF;
    u8 op3 = (instruction >> 12) & 0xF;
    u8 op4 = (instruction >> 16) & 0xF;

    bool x = instruction & (1 << 5);
    bool y = instruction & (1 << 6);

    s16 result1;
    s16 result2;

    if (x) {
        result1 = (s16)(regs.r[op1] >> 16);
    } else {
        result1 = (s16)regs.r[op1];
    }

    if (y) {
        result2 = (s16)(regs.r[op2] >> 16);
    } else {
        result2 = (s16)regs.r[op2];
    }

    u32 result = result1 * result2;

    if (accumulate) {
        u32 operand = regs.r[op3];

        regs.r[op4] = result + operand;

        if ((~(result ^ operand) & (operand ^ regs.r[op4])) >> 31) {
            SetConditionFlag(Q_FLAG, true);
        }
    } else {
        regs.r[op4] = result;
    }

    regs.r[15] += 4;
}

void arm_signed_halfword_word_multiply() {
    if (arch == Arch::ARMv4) {
        return;
    }

    u8 op1 = instruction & 0xF;
    u8 op2 = (instruction >> 8) & 0xF;
    u8 op3 = (instruction >> 12) & 0xF;
    u8 op4 = (instruction >> 16) & 0xF;

    bool x = instruction & (1 << 5);
    bool y = instruction & (1 << 6);

    u32 result;

    if (y) {
        result = ((s32)regs.r[op1] * (s16)(regs.r[op2] >> 16)) >> 16;
    } else {
        result = ((s32)regs.r[op1] * (s16)regs.r[op2]) >> 16;
    }

    if (!x) {
        u32 operand = regs.r[op3];

        regs.r[op4] = result + operand;

        if ((~(result ^ operand) & (operand ^ regs.r[op4])) >> 31) {
            SetConditionFlag(Q_FLAG, true);
        }
    } else {
        regs.r[op4] = result;
    }

    regs.r[15] += 4;
}

void arm_signed_halfword_accumulate_long() {
    if (arch == Arch::ARMv4) {
        return;
    }

    u8 op1 = instruction & 0xF;
    u8 op2 = (instruction >> 8) & 0xF;
    u8 op3 = (instruction >> 12) & 0xF;
    u8 op4 = (instruction >> 16) & 0xF;

    bool x = instruction & (1 << 5);
    bool y = instruction & (1 << 6);

    s64 rdhilo = (s64)(((u64)regs.r[op4] << 32) | ((u64)regs.r[op3]));

    s64 result1;
    s64 result2;

    if (x) {
        result1 = (s64)(s16)(regs.r[op1] >> 16);
    } else {
        result1 = (s64)(s16)regs.r[op1];
    }

    if (y) {
        result2 = (s64)(s16)(regs.r[op2] >> 16);
    } else {
        result2 = (s64)(s16)regs.r[op2];
    }

    s64 result = result1 * result2;
    result += rdhilo;
    regs.r[op3] = result;
    regs.r[op4] = result >> 32;

    regs.r[15] += 4;
}