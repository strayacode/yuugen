u32 ARM_DATA_PROCESSING_IMM() {
    u32 immediate = instruction & 0xFF;
    u8 shift_amount = ((instruction >> 8) & 0xF) << 1;

    u32 result = (immediate >> shift_amount) | (immediate << (32 - shift_amount));
    return result;
}

u32 ARM_DATA_PROCESSING_IMMS() {
    u32 immediate = instruction & 0xFF;
    u8 shift_amount = ((instruction >> 8) & 0xF) << 1;

    u32 result = (immediate >> shift_amount) | (immediate << (32 - shift_amount));
    
    if (shift_amount != 0) {
        SetConditionFlag(C_FLAG, result >> 31);
    }
    return result;
}

INSTRUCTION(ARM_MOV, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    regs.r[rd] = op2;

    if (rd == 15) {
        // word align first
        regs.r[15] &= ~3;

        ARMFlushPipeline();
    } else {
        regs.r[15] += 4;
    }
}

INSTRUCTION(ARM_MOVS, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    regs.r[rd] = op2;

    if (rd == 15) {
        // store the current spsr in cpsr

        UpdateMode(regs.spsr & 0x1F);

        
        regs.cpsr = regs.spsr;

        if (IsARM()) {
            // word align first
            regs.r[15] &= ~3;

            ARMFlushPipeline();
        } else {
            // halfword align and then flush the pipeline
            regs.r[15] &= ~1;

            ThumbFlushPipeline();
        }
    } else {
        SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
        SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
        // c flag changed from shifter carry out

        regs.r[15] += 4;
    }
}

INSTRUCTION(ARM_MVN, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    regs.r[rd] = ~op2;

    if (rd == 15) {
        // // word align first
        // regs.r[15] &= ~3;

        // ARMFlushPipeline();
        log_fatal("handle");
    } else {
        regs.r[15] += 4;
    }
}

INSTRUCTION(ARM_CMPS, u32 op2) {
    // if rn == 15 and I=0 and R=1 (shift by register) then rn which is r15 is read as current instruction address + 12
    u32 rn = regs.r[(instruction >> 16) & 0xF] + (((instruction & 0x20F0010) == 0xF0010) ? 4 : 0);

    u32 result = rn - op2;
    // set flags
    SetConditionFlag(N_FLAG, result >> 31);
    SetConditionFlag(Z_FLAG, result == 0);
    SetConditionFlag(C_FLAG, SUB_CARRY(rn, op2));
    SetConditionFlag(V_FLAG, SUB_OVERFLOW(rn, op2, result));
    regs.r[15] += 4;
}

INSTRUCTION(ARM_CMNS, u32 op2) {
    u8 rn = (instruction >> 16) & 0xF;
    u32 result = regs.r[rn] + op2;

    SetConditionFlag(Z_FLAG, result == 0);
    SetConditionFlag(N_FLAG, result >> 31);
    SetConditionFlag(C_FLAG, ADD_CARRY(regs.r[rn], op2));
    SetConditionFlag(V_FLAG, ADD_OVERFLOW(regs.r[rn], op2, result));

    regs.r[15] += 4;
}

INSTRUCTION(ARM_TSTS, u32 op2) {
    u8 rn = (instruction >> 16) & 0xF;
    u32 result = regs.r[rn] & op2;

    SetConditionFlag(Z_FLAG, result == 0);
    SetConditionFlag(N_FLAG, result >> 31);

    // c flag is set by shifter output

    regs.r[15] += 4;
}

INSTRUCTION(ARM_ADD, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    regs.r[rd] = regs.r[rn] + op2;
    if (rd == 15) {
        log_fatal("handle");
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_ADDS, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    u64 result64 = (u64)regs.r[rn] + (u64)op2;
    regs.r[rd] = regs.r[rn] + op2;
    if (rd == 15) {
        log_fatal("handle");
    } else {
        SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
        SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
        SetConditionFlag(C_FLAG, result64 >> 32);
        SetConditionFlag(V_FLAG, ADD_OVERFLOW(regs.r[rn], op2, regs.r[rd]));
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_SWP) {
    u8 rm = instruction & 0xF;
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;

    u32 address = regs.r[rn];
    u32 data = ReadWord(address);
    if (address & 0x3) {
        int shift_amount = (address & 0x3) * 8;
        data = (data << (32 - shift_amount)) | (data >> shift_amount);
    }

    WriteWord(address, regs.r[rm]);
    regs.r[rd] = data;


    regs.r[15] += 4;
}

INSTRUCTION(ARM_SWPB) {
    u8 rm = instruction & 0xF;
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;

    u8 data = ReadByte(regs.r[rn]);
    WriteByte(regs.r[rn], regs.r[rm]);
    regs.r[rd] = data;

    regs.r[15] += 4;
}

INSTRUCTION(ARM_ADC, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;

    regs.r[rd] = regs.r[rn] + op2 + GetConditionFlag(C_FLAG);
    
    if (rd == 15) {
        log_fatal("handle");
    } 

    regs.r[15] += 4;
}

INSTRUCTION(ARM_ADCS, u32 op2) {
    // u8 rd = (instruction >> 12) & 0xF;
 //    u8 rn = (instruction >> 16) & 0xF;
 //    u32 temporary_result = regs.r[rn] + op2;
 //    regs.r[rd] = temporary_result + GetConditionFlag(C_FLAG);
    
 //    if (rd == 15) {
 //     log_fatal("handle");
 //    } else {
 //     SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
 //     SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
 //     // maybe look at how c flag is accounted for
 //     SetConditionFlag(C_FLAG, ADD_CARRY(regs.r[rn], op2) | ADD_CARRY(temporary_result, GetConditionFlag(C_FLAG)));
 //     SetConditionFlag(V_FLAG, ADD_OVERFLOW(regs.r[rn], op2, temporary_result) | ADD_OVERFLOW(temporary_result, GetConditionFlag(C_FLAG), regs.r[rd]));
 //    }

 //    regs.r[15] += 4;
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    u64 result64 = (u64)regs.r[rn] + (u64)op2 + (u64)GetConditionFlag(C_FLAG);
    u32 result32 = (u32)result64;
    regs.r[rd] = regs.r[rn] + op2 + GetConditionFlag(C_FLAG);
    SetConditionFlag(C_FLAG, result64 >> 32);

    SetConditionFlag(Z_FLAG, result32 == 0);
    SetConditionFlag(N_FLAG, result32 >> 31);

    SetConditionFlag(V_FLAG, (~(regs.r[rn] ^ op2) & (op2 ^ result32)) >> 31);
    

    regs.r[15] += 4;
}

INSTRUCTION(ARM_SUB, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    regs.r[rd] = regs.r[rn] - op2;
    
    if (rd == 15) {
        log_fatal("handle");
    }

    regs.r[15] += 4;
}


INSTRUCTION(ARM_RSB, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    regs.r[rd] = op2 - regs.r[rn];
    
    if (rd == 15) {
        log_fatal("handle");
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_RSBS, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    regs.r[rd] = op2 - regs.r[rn];
    
    if (rd == 15) {
        log_fatal("handle");
    } else {
        SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
        SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
        SetConditionFlag(C_FLAG, SUB_CARRY(op2, regs.r[rn]));
        SetConditionFlag(V_FLAG, SUB_OVERFLOW(op2, regs.r[rn], regs.r[rd]));
        regs.r[15] += 4;
    } 
}

INSTRUCTION(ARM_ORR, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;

    regs.r[rd] = regs.r[rn] | op2;

    if (rd == 15) {
        log_fatal("handle");
    } 

    regs.r[15] += 4;
}

INSTRUCTION(ARM_ORRS, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;

    regs.r[rd] = regs.r[rn] | op2;

    if (rd == 15) {
        log_fatal("handle");
    } else {
        SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
        SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
        // set c flag from shifter carry out
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_SUBS, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    regs.r[rd] = regs.r[rn] - op2;
    
    if (rd == 15) {
        // store the current spsr in cpsr

        UpdateMode(regs.spsr & 0x1F);

        
        regs.cpsr = regs.spsr;

        if (IsARM()) {
            // word align first
            regs.r[15] &= ~3;

            ARMFlushPipeline();
        } else {
            // halfword align and then flush the pipeline
            regs.r[15] &= ~1;

            ThumbFlushPipeline();
        }
    }

    SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
    SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
    SetConditionFlag(C_FLAG, SUB_CARRY(regs.r[rn], op2));
    SetConditionFlag(V_FLAG, SUB_OVERFLOW(regs.r[rn], op2, regs.r[rd]));

    regs.r[15] += 4;
}

INSTRUCTION(ARM_BIC, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    regs.r[rd] = regs.r[rn] & ~op2;
    if (rd == 15) {
        log_fatal("shit");
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_BICS, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    regs.r[rd] = regs.r[rn] & ~op2;
    if (rd == 15) {
        log_fatal("shit");
    } else {
        SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
        SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
        // C flag is set shifter_carry_out
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_AND, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    regs.r[rd] = regs.r[rn] & op2;
    if (rd == 15) {
        log_fatal("handle");
    } 

    regs.r[15] += 4;
}

INSTRUCTION(ARM_EOR, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    regs.r[rd] = regs.r[rn] ^ op2;
    if (rd == 15) {
        log_fatal("handle");
    } 

    regs.r[15] += 4;
}

INSTRUCTION(ARM_EORS, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    regs.r[rd] = regs.r[rn] ^ op2;
    if (rd == 15) {
        log_fatal("handle");
    } else {
        SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
        SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
        // c flag is changed by shifter carry out
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_ANDS, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    regs.r[rd] = regs.r[rn] & op2;
    if (rd == 15) {
        log_fatal("handle");
    } else {
        SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
        SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
        // c flag is set by shifter operand out
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_RSCS, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    regs.r[rd] = op2 - regs.r[rn] - !GetConditionFlag(C_FLAG);
    if (rd == 15) {
        log_fatal("handle");
    } else {
        SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
        SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
        // TODO: check how carry flag is used in calculation later
        SetConditionFlag(C_FLAG, SUB_CARRY(op2, regs.r[rn]));
        SetConditionFlag(V_FLAG, SUB_OVERFLOW(op2, regs.r[rn], regs.r[rd]));
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_SBCS, u32 op2) {
    // u8 rd = (instruction >> 12) & 0xF;
 //    u8 rn = (instruction >> 16) & 0xF;
 //    u32 temporary_result = regs.r[rn] - op2;
 //    regs.r[rd] = regs.r[rn] - op2 - !GetConditionFlag(C_FLAG);
 //    if (rd == 15) {
 //        log_fatal("handle");
 //    } else {
 //        SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
 //        SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
 //        // TODO: check how carry flag is used in calculation later
 //        SetConditionFlag(C_FLAG, SUB_CARRY(regs.r[rn], op2) & SUB_CARRY(temporary_result, !GetConditionFlag(C_FLAG)));
 //        SetConditionFlag(V_FLAG, SUB_OVERFLOW(regs.r[rn], op2, temporary_result) | SUB_OVERFLOW(temporary_result, !GetConditionFlag(C_FLAG), regs.r[rd]));
 //    }

 //    regs.r[15] += 4;

    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    u32 result = regs.r[rn] - op2 - !GetConditionFlag(C_FLAG);

    if (rd == 15) {
        log_fatal("handle");
        // regs.cpsr = get_spsr();
    } else {
        SetConditionFlag(N_FLAG, result >> 31);
        SetConditionFlag(Z_FLAG, result == 0);
        SetConditionFlag(V_FLAG, ((regs.r[rn] ^ op2) & (op2 ^ result)) >> 31);
        // TODO: fix later
        SetConditionFlag(C_FLAG, regs.r[rn] >= result);
    }

    regs.r[rd] = result;

    regs.r[15] += 4;
}

INSTRUCTION(ARM_MLAS) {
    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rn = (instruction >> 12) & 0xF;
    u8 rd = (instruction >> 16) & 0xF;

    regs.r[rd] = (regs.r[rm] * regs.r[rs]) + regs.r[rn];
    SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
    SetConditionFlag(Z_FLAG, regs.r[rd] == 0);

    regs.r[15] += 4;
}

INSTRUCTION(ARM_MULS) {
    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rd = (instruction >> 16) & 0xF;

    regs.r[rd] = regs.r[rm] * regs.r[rs];
    SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
    SetConditionFlag(Z_FLAG, regs.r[rd] == 0);

    regs.r[15] += 4;
}

INSTRUCTION(ARM_UMULLS) {
    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rdlo = (instruction >> 12) & 0xF;
    u8 rdhi = (instruction >> 16) & 0xF;
    u64 result = (u64)regs.r[rm] * (u64)regs.r[rs];
    SetConditionFlag(N_FLAG, result >> 63);
    SetConditionFlag(Z_FLAG, result == 0);

    regs.r[rdhi] = result >> 32;
    regs.r[rdlo] = result & 0xFFFFFFFF;

    regs.r[15] += 4;
}

INSTRUCTION(ARM_SMULLS) {
    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rdlo = (instruction >> 12) & 0xF;
    u8 rdhi = (instruction >> 16) & 0xF;
    s64 result = (s32)(regs.r[rm]);
    result *= (s32)(regs.r[rs]);
    regs.r[rdhi] = result >> 32;
    regs.r[rdlo] = result;
    SetConditionFlag(N_FLAG, regs.r[rdhi] >> 31);
    SetConditionFlag(Z_FLAG, result == 0);
    

    regs.r[15] += 4;
}

INSTRUCTION(ARM_UMLALS) {
    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rdlo = (instruction >> 12) & 0xF;
    u8 rdhi = (instruction >> 16) & 0xF;
    u64 rdhilo = ((u64)regs.r[rdhi] << 32) | ((u64)regs.r[rdlo]);
    u64 result = (u64)regs.r[rm] * (u64)regs.r[rs] + rdhilo;
    SetConditionFlag(N_FLAG, result >> 63);
    SetConditionFlag(Z_FLAG, result == 0);
    regs.r[rdlo] = result & 0xFFFFFFFF;

    regs.r[rdhi] = result >> 32;


    regs.r[15] += 4;
}

INSTRUCTION(ARM_SMLALS) {
    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rdlo = (instruction >> 12) & 0xF;
    u8 rdhi = (instruction >> 16) & 0xF;
    s64 rdhilo = (s64)(((u64)regs.r[rdhi] << 32) | ((u64)regs.r[rdlo]));
    s64 result = ((s64)(s32)regs.r[rm] * (s64)(s32)regs.r[rs]);
    result += rdhilo;
    regs.r[rdlo] = result;
    regs.r[rdhi] = result >> 32;
    SetConditionFlag(N_FLAG, regs.r[rdhi] >> 31);
    SetConditionFlag(Z_FLAG, result == 0);

    regs.r[15] += 4;
}

INSTRUCTION(ARM_CLZ) {
    // armv5 specific
    if (arch == ARMv4) {
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

INSTRUCTION(ARM_QADD) {
    // ARMv5 exclusive
    if (arch == ARMv4) {
        return;
    }

    u8 rm = instruction & 0xF;
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;

    u32 result = regs.r[rm] + regs.r[rn];

    if (ADD_OVERFLOW(regs.r[rm], regs.r[rn], result)) {
        // set q flag
        SetConditionFlag(Q_FLAG, true);

        // since a signed overflow occured with saturated arithmetic, we set the result to the max value
        // according to if its the max negative value (-2^31, 0x80000000) or positive value (2^31 - 1, 0x7FFFFFFF)
        // if greater than the largest positive value (2^31 - 1)
        if (result & 0x80000000) {
            // then set to 2^31 - 1
            result = 0x7FFFFFFF;
        } else {
            // this implies that we overflowed above the largest negative value (-2^31)
            // then set to -2^31
            result = 0x80000000;
        }
    }

    regs.r[rd] = result;

    regs.r[15] += 4;
}

INSTRUCTION(ARM_SMLABB) {
    // ARMv5 exclusive
    if (arch == ARMv4) {
        return;
    }

    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rn = (instruction >> 12) & 0xF;
    u8 rd = (instruction >> 16) & 0xF;

    // signed multiply first bottom half of first operand with signed
    // bottom half of second half and accumulate

    u32 result = (s16)regs.r[rm] * (s16)regs.r[rs];
    regs.r[rd] = result + regs.r[rn];


    if (ADD_OVERFLOW(result, regs.r[rn], regs.r[rd])) {
        SetConditionFlag(Q_FLAG, true);
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_SMLABT) {
    // ARMv5 exclusive
    if (arch == ARMv4) {
        return;
    }

    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rn = (instruction >> 12) & 0xF;
    u8 rd = (instruction >> 16) & 0xF;

    // signed multiply first bottom half of first operand with signed
    // top half of second half and accumulate

    u32 result = (s16)regs.r[rm] * (s16)(regs.r[rs] >> 16);
    regs.r[rd] = result + regs.r[rn];

    if (ADD_OVERFLOW(result, regs.r[rn], regs.r[rd])) {
        SetConditionFlag(Q_FLAG, true);
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_SMLATB) {
    // ARMv5 exclusive
    if (arch == ARMv4) {
        return;
    }

    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rn = (instruction >> 12) & 0xF;
    u8 rd = (instruction >> 16) & 0xF;

    // signed multiply first bottom half of first operand with signed
    // top half of second half and accumulate

    u32 result = (s16)(regs.r[rm] >> 16) * (s16)regs.r[rs];
    regs.r[rd] = result + regs.r[rn];

    if (ADD_OVERFLOW(result, regs.r[rn], regs.r[rd])) {
        SetConditionFlag(Q_FLAG, true);
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_SMLATT) {
    // ARMv5 exclusive
    if (arch == ARMv4) {
        return;
    }

    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rn = (instruction >> 12) & 0xF;
    u8 rd = (instruction >> 16) & 0xF;

    // signed multiply first bottom half of first operand with signed
    // top half of second half and accumulate

    u32 result = (s16)(regs.r[rm] >> 16) * (s16)(regs.r[rs] >> 16);
    regs.r[rd] = result + regs.r[rn];

    if (ADD_OVERFLOW(result, regs.r[rn], regs.r[rd])) {
        SetConditionFlag(Q_FLAG, true);
    }

    regs.r[15] += 4;
}

// shifts
u32 ARM_LOGICAL_SHIFT_LEFT_IMM() {
    u8 shift_amount = (instruction >> 7) & 0x1F;
    u8 rm = instruction & 0xF;
    return regs.r[rm] << shift_amount;
}

u32 ARM_LOGICAL_SHIFT_LEFT_IMMS() {
    u8 shift_amount = (instruction >> 7) & 0x1F;
    u8 rm = instruction & 0xF;

    if (shift_amount > 0) {
        // shifter_carry_out (c flag) = rm[32 - shift_amount]
        SetConditionFlag(C_FLAG, regs.r[rm] & (1 << (32 - shift_amount)));
    }

    return regs.r[rm] << shift_amount;
}

u32 ARM_LOGICAL_SHIFT_LEFT_REGS() {
    u8 shift_amount = regs.r[(instruction >> 8) & 0xF] & 0xFF;
    u32 rm = regs.r[instruction & 0xF] + (((instruction & 0xF) == 15) ? 4 : 0);
    
    u32 result = 0;
    if (shift_amount == 0) {
        result = rm;
    } else if (shift_amount < 32) {
        result = rm << shift_amount;
        SetConditionFlag(C_FLAG, rm & (1 << (32 - shift_amount)));
    } else if (shift_amount == 32) {
        SetConditionFlag(C_FLAG, rm & 0x1);
    } else {
        // shift amount > 32
        SetConditionFlag(C_FLAG, false);
    }

    return result;
}

u32 ARM_LOGICAL_SHIFT_RIGHT_IMM() {
    u8 shift_amount = (instruction >> 7) & 0x1F;
    u8 rm = instruction & 0xF;
    return regs.r[rm] >> shift_amount;
}

u32 ARM_LOGICAL_SHIFT_RIGHT_IMMS() {
    u8 shift_amount = (instruction >> 7) & 0x1F;
    u8 rm = instruction & 0xF;

    u32 result = 0;

    if (shift_amount == 0) {
        SetConditionFlag(C_FLAG, regs.r[rm] >> 31);
    } else {
        result = regs.r[rm] >> shift_amount;
        SetConditionFlag(C_FLAG, regs.r[rm] && (1 << (shift_amount - 1)));
    }

    return result;
}

u32 ARM_ARITHMETIC_SHIFT_RIGHT_IMMS() {
    u8 shift_amount = (instruction >> 7) & 0x1F;
    u8 rm = instruction & 0xF;
    u32 result = 0;
    u8 msb = regs.r[rm] >> 31;

    if (shift_amount == 0) {
        result = 0xFFFFFFFF * msb;
        SetConditionFlag(C_FLAG, msb);
    } else {
        // shift amount > 0
        SetConditionFlag(C_FLAG, regs.r[rm] & (1 << (shift_amount - 1)));
        // perform asr
        // what happens is that rm gets shifted normally to the right but then the bits not set are set with the msb
        result = (regs.r[rm] >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
    }
    return result;
}

u32 ARM_ARITHMETIC_SHIFT_RIGHT_IMM() {
    u8 shift_amount = (instruction >> 7) & 0x1F;
    u8 rm = instruction & 0xF;
    u32 result = 0;
    u8 msb = regs.r[rm] >> 31;

    if (shift_amount == 0) {
        result = 0xFFFFFFFF * msb;
    } else {
        // shift amount > 0
        // perform asr
        // what happens is that rm gets shifted normally to the right but then the bits not set are set with the msb
        result = (regs.r[rm] >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
    }
    return result;
}

u32 ARM_ARITHMETIC_SHIFT_RIGHT_REGS() {
    u8 shift_amount = regs.r[(instruction >> 8) & 0xF] & 0xFF;
    u32 rm = regs.r[instruction & 0xF];
    if ((instruction & 0xF) == 15) {
        log_fatal("handle");
        // prefetch due to how register shifting executes after the prefetch
        rm += 4;
    }
    u32 result = 0;
    u8 msb = rm >> 31;

    if (shift_amount == 0) {
        result = rm;
    } else if (shift_amount < 32) {
        // shift amount > 0
        SetConditionFlag(C_FLAG, rm & (1 << (shift_amount - 1)));
        // perform asr
        // what happens is that rm gets shifted normally to the right but then the bits not set are set with the msb
        result = (rm >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
    } else {
        // shift amount > 32
        result = 0xFFFFFFFF * msb;
        SetConditionFlag(C_FLAG, msb);
    }
    return result;
}

u32 ARM_ROTATE_RIGHT_IMMS() {
    u8 shift_amount = (instruction >> 7) & 0x1F;
    u8 rm = instruction & 0xF;

    u32 result = 0;
    if (shift_amount == 0) {
        // perform rotate right extend
        result = (GetConditionFlag(C_FLAG) << 31) | (regs.r[rm] >> 1);
        SetConditionFlag(C_FLAG, regs.r[rm] & 0x1);
    } else {
        // shift amount > 0
        result = (regs.r[rm] >> shift_amount) | (regs.r[rm] << (32 - shift_amount));
        SetConditionFlag(C_FLAG, regs.r[rm] & (1 << (shift_amount - 1)));
    }

    return result;
}