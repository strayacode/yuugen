INSTRUCTION(THUMB_MOV_IMM) {
    u8 rd = (instruction >> 8) & 0x7;

    regs.r[rd] = instruction & 0xFF;
    
    // set flags
    SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
    SetConditionFlag(Z_FLAG, regs.r[rd] == 0);

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_SUB_IMM) {
    u32 immediate = instruction & 0xFF;
    u8 rd = (instruction >> 8) & 0x7;
    
    u32 result = regs.r[rd] - immediate;
    
    SetConditionFlag(N_FLAG, result >> 31);
    SetConditionFlag(Z_FLAG, result == 0);
    SetConditionFlag(C_FLAG, SUB_CARRY(regs.r[rd], immediate));
    SetConditionFlag(V_FLAG, SUB_OVERFLOW(regs.r[rd], immediate, result));

    regs.r[rd] = result;

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_ADD_IMM) {
    u32 immediate = instruction & 0xFF;
    u8 rd = (instruction >> 8) & 0x7;
    u32 result = regs.r[rd] + immediate;
    
    SetConditionFlag(N_FLAG, result >> 31);
    SetConditionFlag(Z_FLAG, result == 0);
    SetConditionFlag(C_FLAG, ADD_CARRY(regs.r[rd], immediate));
    SetConditionFlag(V_FLAG, ADD_OVERFLOW(regs.r[rd], immediate, result));

    regs.r[rd] = result;

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_ADD_SP_IMM) {
    u32 immediate = (instruction & 0x7F) << 2;

    // need to check bit 7 to check if we subtract or add from sp
    regs.r[13] = regs.r[13] + ((instruction & (1 << 7)) ? - immediate : immediate);

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_CMP_IMM) {
    u8 rn = (instruction >> 8) & 0x7;
    u32 immediate = instruction & 0xFF;
    u32 result = regs.r[rn] - immediate;

    SetConditionFlag(N_FLAG, result >> 31);
    SetConditionFlag(Z_FLAG, result == 0);
    SetConditionFlag(C_FLAG, SUB_CARRY(regs.r[rn], immediate));
    SetConditionFlag(V_FLAG, SUB_OVERFLOW(regs.r[rn], immediate, result));

    regs.r[15] += 2;  
}

INSTRUCTION(THUMB_LSL_IMM) {
    u8 rd = instruction & 0x7;
    u8 rm = (instruction >> 3) & 0x7;
    u32 immediate = (instruction >> 6) & 0x1F;
    // TODO: optimise later

    if (immediate > 0) {
        SetConditionFlag(C_FLAG, regs.r[rm] & (1 << (32 - immediate)));

    }
    
    regs.r[rd] = regs.r[rm] << immediate;
    
    SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
    SetConditionFlag(Z_FLAG, regs.r[rd] == 0);

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_CMP_DATA_PROCESSING) {
    u8 rn = instruction & 0x7;
    u8 rm = (instruction >> 3) & 0x7;
    u32 result = regs.r[rn] - regs.r[rm];

    // set flags
    SetConditionFlag(N_FLAG, result >> 31);
    SetConditionFlag(Z_FLAG, result == 0);
    SetConditionFlag(C_FLAG, SUB_CARRY(regs.r[rn], regs.r[rm]));
    SetConditionFlag(V_FLAG, SUB_OVERFLOW(regs.r[rn], regs.r[rm], result));

    regs.r[15] += 2;  
}

INSTRUCTION(THUMB_ORR_DATA_PROCESSING) {
    u8 rd = instruction & 0x7;
    u8 rm = (instruction >> 3) & 0x7;
    regs.r[rd] |= regs.r[rm];

    SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
    SetConditionFlag(Z_FLAG, regs.r[rd] == 0);


    regs.r[15] += 2;
}

INSTRUCTION(THUMB_ROR_DATA_PROCESSING) {
    u8 rd = instruction & 0x7;
    u8 rs = (instruction >> 3) & 0x7;

    u8 shift_amount = regs.r[rs] & 0x1F;
    // TODO: optimise
    if ((regs.r[rs] & 0xFF) == 0) {
        // do nothing lol
    } else if (shift_amount == 0) {
        SetConditionFlag(C_FLAG, regs.r[rd] >> 31);
    } else {
        // shift amount > 0
        SetConditionFlag(C_FLAG, regs.r[rd] & (1 << (shift_amount - 1)));
        regs.r[rd] = (regs.r[rd] >> shift_amount) | (regs.r[rd] << (32 - shift_amount));
    }
    
    SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
    SetConditionFlag(Z_FLAG, regs.r[rd] == 0);

    

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_MVN_DATA_PROCESSING) {
    u8 rd = instruction & 0x7;
    u8 rm = (instruction >> 3) & 0x7;

    regs.r[rd] = ~regs.r[rm];

    SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
    SetConditionFlag(Z_FLAG, regs.r[rd] == 0);

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_AND_DATA_PROCESSING) {
    u8 rd = instruction & 0x7;
    u8 rm = (instruction >> 3) & 0x7;
    regs.r[rd] &= regs.r[rm];

    SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
    SetConditionFlag(Z_FLAG, regs.r[rd] == 0);


    regs.r[15] += 2;
}

INSTRUCTION(THUMB_EOR_DATA_PROCESSING) {
    u8 rd = instruction & 0x7;
    u8 rm = (instruction >> 3) & 0x7;
    regs.r[rd] ^= regs.r[rm];

    SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
    SetConditionFlag(Z_FLAG, regs.r[rd] == 0);


    regs.r[15] += 2;
}

INSTRUCTION(THUMB_LSR_DATA_PROCESSING) {
    u8 rd = instruction & 0x7;
    u8 rs = (instruction >> 3) & 0x7;

    // TODO: make cleaner
    u8 shift_amount = regs.r[rs] & 0xFF;
    if (shift_amount < 32) {
        SetConditionFlag(C_FLAG, regs.r[rd] && (1 << (shift_amount - 1)));
        regs.r[rd] >>= shift_amount;
    } else if (shift_amount == 32) {
        SetConditionFlag(C_FLAG, regs.r[rd] >> 31);
        regs.r[rd] = 0;
    } else {
        // shift amount > 32
        SetConditionFlag(C_FLAG, false);
        regs.r[rd] = 0;
    }

    SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
    SetConditionFlag(Z_FLAG, regs.r[rd] == 0);

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_ASR_DATA_PROCESSING) {
    u8 shift_amount = regs.r[(instruction >> 3) & 0x7] & 0xFF;
    u8 rd = instruction & 0x7;
    
    u8 msb = regs.r[rd] >> 31;

    if (shift_amount == 0) {
        // unaffected
    } else if (shift_amount < 32) {
        // shift amount > 0
        SetConditionFlag(C_FLAG, regs.r[rd] & (1 << (shift_amount - 1)));
        // perform asr
        // what happens is that rm gets shifted normally to the right but then the bits not set are set with the msb
        regs.r[rd] = (regs.r[rd] >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
    } else {
        // shift amount > 32
        regs.r[rd] = 0xFFFFFFFF * msb;
        SetConditionFlag(C_FLAG, msb);
    }
    
    regs.r[15] += 2;
}


INSTRUCTION(THUMB_LSL_DATA_PROCESSING) {
    u8 rd = instruction & 0x7;
    u8 rs = (instruction >> 3) & 0x7;
    u8 shift_amount = regs.r[rs] & 0xFF;
    // TODO: optimise later
    if (shift_amount == 0) {
        // do nothing lol
    } else if (shift_amount < 32) {
        SetConditionFlag(C_FLAG, regs.r[rd] & (1 << (32 - shift_amount)));
        regs.r[rd] <<= shift_amount;
    } else if (shift_amount == 32) {
        SetConditionFlag(C_FLAG, regs.r[rd] & 0x1);
        regs.r[rd] = 0;
    } else {
        // shift_amount > 32
        SetConditionFlag(C_FLAG, false);
        regs.r[rd] = 0;
    }

    SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
    SetConditionFlag(Z_FLAG, regs.r[rd] == 0);

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_MUL_DATA_PROCESSING) {
    u8 rd = instruction & 0x7;
    u8 rm = (instruction >> 3) & 0x7;

    regs.r[rd] *= regs.r[rm];
    SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
    SetConditionFlag(Z_FLAG, regs.r[rd] == 0);

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_SBC_DATA_PROCESSING) {
    u8 rd = instruction & 0x7;
    u8 rm = (instruction >> 3) & 0x7;
    
    u64 result64 = (u64)regs.r[rd] - (u64)regs.r[rm] - (u64)!GetConditionFlag(C_FLAG);

    u32 result32 = (u32)result64;
    
    

    SetConditionFlag(Z_FLAG, result32 == 0);
    SetConditionFlag(N_FLAG, result32 >> 31);

    SetConditionFlag(V_FLAG, ((regs.r[rd] ^ regs.r[rm]) & (regs.r[rm] ^ result32)) >> 31);
    regs.r[rd] = regs.r[rd] - regs.r[rm] - !GetConditionFlag(C_FLAG);

    SetConditionFlag(C_FLAG, result64 >> 32);

    regs.r[15] += 2;
}


INSTRUCTION(THUMB_MOVH) {
    u8 rd = ((instruction & (1 << 7)) >> 4) | (instruction & 0x7);

    u8 rm = (instruction >> 3) & 0xF;
    regs.r[rd] = regs.r[rm];

    if (rd == 15) {
        log_fatal("handle");
        // halfword align
        regs.r[15] &= ~1;
        ThumbFlushPipeline();
    } else {
        regs.r[15] += 2;    
    } 
}

INSTRUCTION(THUMB_ADDH) {
    u8 rd = ((instruction & (1 << 7)) >> 4) | (instruction & 0x7);
    u8 rm = (instruction >> 3) & 0xF;

    if (rd == 15) {
        log_fatal("handle");
    }

    regs.r[rd] += regs.r[rm];

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_CMPH) {
    u8 rn = ((instruction & (1 << 7)) >> 4) | (instruction & 0x7);
    u8 rm = (instruction >> 3) & 0xF;
    u32 result = regs.r[rn] - regs.r[rm];


    // set flags
    SetConditionFlag(N_FLAG, result >> 31);
    SetConditionFlag(Z_FLAG, result == 0);
    SetConditionFlag(C_FLAG, SUB_CARRY(regs.r[rn], regs.r[rm]));
    SetConditionFlag(V_FLAG, SUB_OVERFLOW(regs.r[rn], regs.r[rm], result));


    regs.r[15] += 2;  
}

INSTRUCTION(THUMB_SUB_IMM3) {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;


    u8 immediate = (instruction >> 6) & 0x7;

    regs.r[rd] = regs.r[rn] - immediate;

    SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
    SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
    SetConditionFlag(C_FLAG, SUB_CARRY(regs.r[rn], immediate));
    SetConditionFlag(V_FLAG, SUB_OVERFLOW(regs.r[rn], immediate, regs.r[rd]));

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_ADD_IMM3) {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;


    u8 immediate = (instruction >> 6) & 0x7;

    regs.r[rd] = regs.r[rn] + immediate;

    SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
    SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
    SetConditionFlag(C_FLAG, ADD_CARRY(regs.r[rn], immediate));
    SetConditionFlag(V_FLAG, ADD_OVERFLOW(regs.r[rn], immediate, regs.r[rd]));

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_ADD_REG) {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;
    u8 rm = (instruction >> 6) & 0x7;
    regs.r[rd] = regs.r[rn] + regs.r[rm];
    SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
    SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
    SetConditionFlag(C_FLAG, ADD_CARRY(regs.r[rn], regs.r[rm]));
    SetConditionFlag(V_FLAG, ADD_OVERFLOW(regs.r[rn], regs.r[rm], regs.r[rd]));

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_ADC_DATA_PROCESSING) {
    u8 rd = instruction & 0x7;
    u8 rm = (instruction >> 3) & 0x7;
    
    u64 result64 = (u64)regs.r[rd] + (u64)regs.r[rm] + (u64)GetConditionFlag(C_FLAG);

    u32 result32 = (u32)result64;
    
    

    SetConditionFlag(Z_FLAG, result32 == 0);
    SetConditionFlag(N_FLAG, result32 >> 31);

    SetConditionFlag(V_FLAG, (~(regs.r[rd] ^ regs.r[rm]) & (regs.r[rm] ^ result32)) >> 31);
    regs.r[rd] = regs.r[rd] + regs.r[rm] + GetConditionFlag(C_FLAG);

    SetConditionFlag(C_FLAG, result64 >> 32);

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_TST_DATA_PROCESSING) {
    u8 rn = instruction & 0x7;
    u8 rm = (instruction >> 3) & 0x7;

    u32 result = regs.r[rn] & regs.r[rm];

    SetConditionFlag(N_FLAG, result >> 31);
    SetConditionFlag(Z_FLAG, result == 0);

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_BIC_DATA_PROCESSING) {
    u8 rd = instruction & 0x7;
    u8 rm = (instruction >> 3) & 0x7;

    regs.r[rd] = regs.r[rd] & ~regs.r[rm];

    if (rd == 15) {
        log_fatal("handle");
    }

    SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
    SetConditionFlag(Z_FLAG, regs.r[rd] == 0);

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_SUB_REG) {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;
    u8 rm = (instruction >> 6) & 0x7;
    regs.r[rd] = regs.r[rn] - regs.r[rm];
    SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
    SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
    SetConditionFlag(C_FLAG, SUB_CARRY(regs.r[rn], regs.r[rm]));
    SetConditionFlag(V_FLAG, SUB_OVERFLOW(regs.r[rn], regs.r[rm], regs.r[rd]));

    regs.r[15] += 2;
}


INSTRUCTION(THUMB_NEG_DATA_PROCESSING) {
    u8 rd = instruction & 0x7;
    u8 rm = (instruction >> 3) & 0x7;
    u8 zero = 0;

    regs.r[rd] = zero - regs.r[rm];

    SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
    SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
    SetConditionFlag(C_FLAG, SUB_CARRY(zero, regs.r[rm]));
    SetConditionFlag(V_FLAG, SUB_OVERFLOW(zero, regs.r[rm], regs.r[rd]));

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_ASR_IMM) {
    u8 rd = instruction & 0x7;
    u8 rm = (instruction >> 3) & 0x7;

    u32 immediate = (instruction >> 6) & 0x1F;

    u32 msb = regs.r[rm] >> 31;

    if (immediate == 0) {
        SetConditionFlag(C_FLAG, msb);
        regs.r[rd] = 0xFFFFFFFF * msb;
    } else {
        // immediate > 0
        SetConditionFlag(C_FLAG, regs.r[rm] & (1 << (immediate - 1)));
        regs.r[rd] = (regs.r[rm] >> immediate) | ((0xFFFFFFFF * msb) << (32 - immediate));
    }

    SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
    SetConditionFlag(Z_FLAG, regs.r[rd] == 0);

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_LSR_IMM) {
    u8 rd = instruction & 0x7;
    u8 rm = (instruction >> 3) & 0x7;
    u32 immediate = (instruction >> 6) & 0x1F;
    if (immediate == 0) {
        SetConditionFlag(C_FLAG, regs.r[rm] >> 31);
        regs.r[rd] = 0;
    } else {
        // immediate_5 > 0
        SetConditionFlag(C_FLAG, regs.r[rm] & (1 << (immediate - 1)));
        regs.r[rd] = regs.r[rm] >> immediate;
        
    }
    SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
    SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
    regs.r[15] += 2;
}

INSTRUCTION(THUMB_ADD_SP_REG) {
    u32 immediate = instruction & 0xFF;
    u8 rd = (instruction >> 8) & 0x7;

    regs.r[rd] = regs.r[13] + (immediate << 2);

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_ADD_PC_REG) {
    u32 immediate = instruction & 0xFF;
    u8 rd = (instruction >> 8) & 0x7;

    regs.r[rd] = (regs.r[15] & ~0x3) + (immediate << 2);

    regs.r[15] += 2;
}