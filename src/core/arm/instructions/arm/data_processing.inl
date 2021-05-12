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
        // store the current spsr in cpsr only if in privileged mode
        if (PrivilegedMode()) {
            u32 current_spsr = GetCurrentSPSR();
            
            SwitchMode(current_spsr & 0x1F);
            regs.cpsr = current_spsr;
        } else {
            log_fatal("[ARM%d] Loading spsr into cpsr in non-privileged mode is undefined behaviour", arch ? 9 : 7);
        }

        if (IsARM()) {
            // word align first
            regs.r[15] &= ~3;

            ARMFlushPipeline();
        } else {
            // halfword align and then flush the pipeline
            regs.r[15] &= ~1;

            ThumbFlushPipeline();
        }
        return;
    } else {
        regs.r[15] += 4;
    }

    SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
    SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
    // c flag changed from shifter carry out
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
    u32 op1 = regs.r[(instruction >> 16) & 0xF] + (((instruction & 0x20F0010) == 0xF0010) ? 4 : 0);

    u32 result = op1 - op2;
    // set flags
    SetConditionFlag(N_FLAG, result >> 31);
    SetConditionFlag(Z_FLAG, result == 0);
    SetConditionFlag(C_FLAG, SUB_CARRY(op1, op2));
    SetConditionFlag(V_FLAG, SUB_OVERFLOW(op1, op2, result));
    regs.r[15] += 4;
}

INSTRUCTION(ARM_CMNS, u32 op2) {
    u32 op1 = regs.r[(instruction >> 16) & 0xF];
    u32 result = op1 + op2;

    SetConditionFlag(Z_FLAG, result == 0);
    SetConditionFlag(N_FLAG, result >> 31);
    SetConditionFlag(C_FLAG, ADD_CARRY(op1, op2));
    SetConditionFlag(V_FLAG, ADD_OVERFLOW(op1, op2, result));

    regs.r[15] += 4;
}

INSTRUCTION(ARM_TSTS, u32 op2) {
    u32 op1 = regs.r[(instruction >> 16) & 0xF];
    u32 result = op1 & op2;

    SetConditionFlag(Z_FLAG, result == 0);
    SetConditionFlag(N_FLAG, result >> 31);

    // c flag is set by shifter output

    regs.r[15] += 4;
}

INSTRUCTION(ARM_ADD, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u32 op1 = regs.r[(instruction >> 16) & 0xF];
    regs.r[rd] = op1 + op2;
    if (rd == 15) {
        // word align first
        regs.r[15] &= ~3;

        ARMFlushPipeline();
    } else {
        regs.r[15] += 4;
    }
}

INSTRUCTION(ARM_ADDS, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u32 op1 = regs.r[(instruction >> 16) & 0xF];
    u64 result64 = (u64)op1 + (u64)op2;
    regs.r[rd] = op1 + op2;
    if (rd == 15) {
        log_fatal("handle");
    } else {
        SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
        SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
        SetConditionFlag(C_FLAG, result64 >> 32);
        SetConditionFlag(V_FLAG, ADD_OVERFLOW(op1, op2, regs.r[rd]));
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_SWP) {
    u8 rm = instruction & 0xF;
    u8 rd = (instruction >> 12) & 0xF;
    u32 op1 = regs.r[(instruction >> 16) & 0xF];

    u32 address = op1;
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
    u32 op1 = regs.r[(instruction >> 16) & 0xF];

    u8 data = ReadByte(op1);
    WriteByte(op1, regs.r[rm]);
    regs.r[rd] = data;

    regs.r[15] += 4;
}

INSTRUCTION(ARM_ADC, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u32 op1 = regs.r[(instruction >> 16) & 0xF];

    regs.r[rd] = op1 + op2 + GetConditionFlag(C_FLAG);
    
    if (rd == 15) {
        log_fatal("handle");
    } 

    regs.r[15] += 4;
}

INSTRUCTION(ARM_ADCS, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u32 op1 = regs.r[(instruction >> 16) & 0xF];
    u64 result64 = (u64)op1 + (u64)op2 + (u64)GetConditionFlag(C_FLAG);
    u32 result32 = (u32)result64;
    regs.r[rd] = op1 + op2 + GetConditionFlag(C_FLAG);
    SetConditionFlag(C_FLAG, result64 >> 32);

    SetConditionFlag(Z_FLAG, result32 == 0);
    SetConditionFlag(N_FLAG, result32 >> 31);

    SetConditionFlag(V_FLAG, (~(op1 ^ op2) & (op2 ^ result32)) >> 31);
    

    regs.r[15] += 4;
}

INSTRUCTION(ARM_SUB, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u32 op1 = regs.r[(instruction >> 16) & 0xF];
    regs.r[rd] = op1 - op2;
    
    if (rd == 15) {
        log_fatal("handle");
    }

    regs.r[15] += 4;
}


INSTRUCTION(ARM_RSB, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u32 op1 = regs.r[(instruction >> 16) & 0xF];
    regs.r[rd] = op2 - op1;
    
    if (rd == 15) {
        log_fatal("handle");
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_RSBS, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u32 op1 = regs.r[(instruction >> 16) & 0xF];
    regs.r[rd] = op2 - op1;
    
    if (rd == 15) {
        log_fatal("handle");
    } else {
        SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
        SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
        SetConditionFlag(C_FLAG, SUB_CARRY(op2, op1));
        SetConditionFlag(V_FLAG, SUB_OVERFLOW(op2, op1, regs.r[rd]));
        regs.r[15] += 4;
    } 
}

INSTRUCTION(ARM_ORR, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u32 op1 = regs.r[(instruction >> 16) & 0xF];

    regs.r[rd] = op1 | op2;

    if (rd == 15) {
        log_fatal("handle");
    } 

    regs.r[15] += 4;
}

INSTRUCTION(ARM_ORRS, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u32 op1 = regs.r[(instruction >> 16) & 0xF];

    regs.r[rd] = op1 | op2;

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
    u32 op1 = regs.r[(instruction >> 16) & 0xF];

    regs.r[rd] = op1 - op2;
    
    if (rd == 15) {
        // store the current spsr in cpsr only if in privileged mode
        if (PrivilegedMode()) {
            u32 current_spsr = GetCurrentSPSR();

            SwitchMode(current_spsr & 0x1F);
            regs.cpsr = current_spsr;
        } else {
            log_fatal("[ARM%d] Loading spsr into cpsr in non-privileged mode is undefined behaviour", arch ? 9 : 7);
        }
        
        if (IsARM()) {
            // word align first
            regs.r[15] &= ~3;

            ARMFlushPipeline();
        } else {
            // halfword align and then flush the pipeline
            regs.r[15] &= ~1;

            ThumbFlushPipeline();
        }
        return;
    } else {
        regs.r[15] += 4;
    }

    SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
    SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
    SetConditionFlag(C_FLAG, SUB_CARRY(op1, op2));
    SetConditionFlag(V_FLAG, SUB_OVERFLOW(op1, op2, regs.r[rd]));   
}

INSTRUCTION(ARM_BIC, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u32 op1 = regs.r[(instruction >> 16) & 0xF];
    regs.r[rd] = op1 & ~op2;
    if (rd == 15) {
        log_fatal("shit");
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_BICS, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u32 op1 = regs.r[(instruction >> 16) & 0xF];
    regs.r[rd] = op1 & ~op2;
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
    u32 op1 = regs.r[(instruction >> 16) & 0xF];
    regs.r[rd] = op1 & op2;
    if (rd == 15) {
        log_fatal("handle");
    } 

    regs.r[15] += 4;
}

INSTRUCTION(ARM_EOR, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u32 op1 = regs.r[(instruction >> 16) & 0xF];
    regs.r[rd] = op1 ^ op2;
    if (rd == 15) {
        log_fatal("handle");
    } 

    regs.r[15] += 4;
}

INSTRUCTION(ARM_EORS, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u32 op1 = regs.r[(instruction >> 16) & 0xF];
    regs.r[rd] = op1 ^ op2;
    if (rd == 15) {
        log_fatal("handle");
    } else {
        SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
        SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
        // c flag is changed by shifter carry out
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_TEQS, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u32 op1 = regs.r[(instruction >> 16) & 0xF];
    u32 result = op1 ^ op2;
    if (rd == 15) {
        log_fatal("handle");
    } else {
        SetConditionFlag(N_FLAG, result >> 31);
        SetConditionFlag(Z_FLAG, result == 0);
        // c flag is changed by shifter carry out
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_ANDS, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u32 op1 = regs.r[(instruction >> 16) & 0xF];
    regs.r[rd] = op1 & op2;
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
    u32 op1 = regs.r[(instruction >> 16) & 0xF];
    regs.r[rd] = op2 - op1 - !GetConditionFlag(C_FLAG);
    if (rd == 15) {
        log_fatal("handle");
    } else {
        SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
        SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
        // TODO: check how carry flag is used in calculation later
        SetConditionFlag(C_FLAG, SUB_CARRY(op2, op1));
        SetConditionFlag(V_FLAG, SUB_OVERFLOW(op2, op1, regs.r[rd]));
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_SBC, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u32 op1 = regs.r[(instruction >> 16) & 0xF];
    u32 result = op1 - op2 - !GetConditionFlag(C_FLAG);

    if (rd == 15) {
        log_fatal("handle");
    }

    regs.r[rd] = result;

    regs.r[15] += 4;
}

INSTRUCTION(ARM_SBCS, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u32 op1 = regs.r[(instruction >> 16) & 0xF];
    u32 result = op1 - op2 - !GetConditionFlag(C_FLAG);

    if (rd == 15) {
        log_fatal("handle");
    } else {
        SetConditionFlag(N_FLAG, result >> 31);
        SetConditionFlag(Z_FLAG, result == 0);
        SetConditionFlag(V_FLAG, SUB_OVERFLOW(op1, op2, result));
        // TODO: fix later
        SetConditionFlag(C_FLAG, SUB_CARRY(op1, op2) & SUB_CARRY(op1 - op2, !GetConditionFlag(C_FLAG)));
    }

    regs.r[rd] = result;

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
    u32 op1 = regs.r[(instruction >> 16) & 0xF];

    u32 result = regs.r[rm] + op1;

    if (ADD_OVERFLOW(regs.r[rm], op1, result)) {
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

INSTRUCTION(ARM_QSUB) {
    // ARMv5 exclusive
    if (arch == ARMv4) {
        return;
    }

    u8 rm = instruction & 0xF;
    u8 rd = (instruction >> 12) & 0xF;
    u32 op1 = regs.r[(instruction >> 16) & 0xF];

    u32 result = regs.r[rm] - op1;

    if (SUB_OVERFLOW(regs.r[rm], op1, result)) {
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