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

INSTRUCTION(ARM_MLA) {
    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rn = (instruction >> 12) & 0xF;
    u8 rd = (instruction >> 16) & 0xF;

    regs.r[rd] = (regs.r[rm] * regs.r[rs]) + regs.r[rn];
    
    regs.r[15] += 4;
}

INSTRUCTION(ARM_MUL) {
    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rd = (instruction >> 16) & 0xF;

    regs.r[rd] = regs.r[rm] * regs.r[rs];

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

INSTRUCTION(ARM_UMULL) {
    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rdlo = (instruction >> 12) & 0xF;
    u8 rdhi = (instruction >> 16) & 0xF;
    u64 result = (u64)regs.r[rm] * (u64)regs.r[rs];

    regs.r[rdhi] = result >> 32;
    regs.r[rdlo] = result & 0xFFFFFFFF;

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

INSTRUCTION(ARM_SMLABB) {
    // ARMv5 exclusive
    if (arch == ARMv4) {
        return;
    }

    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u32 op1 = regs.r[(instruction >> 12) & 0xF];
    u8 rd = (instruction >> 16) & 0xF;

    // signed multiply first bottom half of first operand with signed
    // bottom half of second half and accumulate

    u32 result = (s16)regs.r[rm] * (s16)regs.r[rs];
    regs.r[rd] = result + op1;


    if (ADD_OVERFLOW(result, op1, regs.r[rd])) {
        SetConditionFlag(Q_FLAG, true);
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_SMULBB) {
    // ARMv5 exclusive
    if (arch == ARMv4) {
        return;
    }

    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rd = (instruction >> 16) & 0xF;

    regs.r[rd] = (s16)regs.r[rm] * (s16)regs.r[rs];

    regs.r[15] += 4;
}

INSTRUCTION(ARM_SMULTB) {
    // ARMv5 exclusive
    if (arch == ARMv4) {
        return;
    }

    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rd = (instruction >> 16) & 0xF;

    regs.r[rd] = (s16)(regs.r[rm] >> 16) * (s16)regs.r[rs];

    regs.r[15] += 4;
}

INSTRUCTION(ARM_SMULBT) {
    // ARMv5 exclusive
    if (arch == ARMv4) {
        return;
    }

    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rd = (instruction >> 16) & 0xF;

    regs.r[rd] = (s16)regs.r[rm] * (s16)(regs.r[rs] >> 16);
    
    regs.r[15] += 4;
}

INSTRUCTION(ARM_SMULTT) {
    // ARMv5 exclusive
    if (arch == ARMv4) {
        return;
    }

    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rd = (instruction >> 16) & 0xF;

    regs.r[rd] = (s16)(regs.r[rm] >> 16) * (s16)(regs.r[rs] >> 16);
    
    regs.r[15] += 4;
}

INSTRUCTION(ARM_SMLABT) {
    // ARMv5 exclusive
    if (arch == ARMv4) {
        return;
    }

    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u32 op1 = regs.r[(instruction >> 12) & 0xF];
    u8 rd = (instruction >> 16) & 0xF;

    // signed multiply first bottom half of first operand with signed
    // top half of second half and accumulate

    u32 result = (s16)regs.r[rm] * (s16)(regs.r[rs] >> 16);
    regs.r[rd] = result + op1;

    if (ADD_OVERFLOW(result, op1, regs.r[rd])) {
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
    u32 op1 = regs.r[(instruction >> 12) & 0xF];
    u8 rd = (instruction >> 16) & 0xF;

    // signed multiply first bottom half of first operand with signed
    // top half of second half and accumulate

    u32 result = (s16)(regs.r[rm] >> 16) * (s16)regs.r[rs];
    regs.r[rd] = result + op1;

    if (ADD_OVERFLOW(result, op1, regs.r[rd])) {
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
    u32 op1 = regs.r[(instruction >> 12) & 0xF];
    u8 rd = (instruction >> 16) & 0xF;

    // signed multiply first bottom half of first operand with signed
    // top half of second half and accumulate
    u32 result = (s16)(regs.r[rm] >> 16) * (s16)(regs.r[rs] >> 16);
    regs.r[rd] = result + op1;

    if (ADD_OVERFLOW(result, op1, regs.r[rd])) {
        SetConditionFlag(Q_FLAG, true);
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_SMULWB) {
    // ARMv5 exclusive
    if (arch == ARMv4) {
        return;
    }

    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rd = (instruction >> 16) & 0xF;

    regs.r[rd] = ((s32)regs.r[rm] * (s16)regs.r[rs]) >> 16;

    regs.r[15] += 4;
}

INSTRUCTION(ARM_SMULWT) {
    // ARMv5 exclusive
    if (arch == ARMv4) {
        return;
    }

    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rd = (instruction >> 16) & 0xF;

    regs.r[rd] = ((s32)regs.r[rm] * (s16)(regs.r[rs] >> 16)) >> 16;

    regs.r[15] += 4;
}

INSTRUCTION(ARM_SMLAWB) {
    // ARMv5 exclusive
    if (arch == ARMv4) {
        return;
    }

    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u32 op1 = regs.r[(instruction >> 12) & 0xF];
    u8 rd = (instruction >> 16) & 0xF;

    u32 result = ((s32)regs.r[rm] * (s16)regs.r[rs]) >> 16;
    regs.r[rd] = result + op1;

    if (ADD_OVERFLOW(result, op1, regs.r[rd])) {
        SetConditionFlag(Q_FLAG, true);
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_SMLAWT) {
    // ARMv5 exclusive
    if (arch == ARMv4) {
        return;
    }

    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u32 op1 = regs.r[(instruction >> 12) & 0xF];
    u8 rd = (instruction >> 16) & 0xF;

    u32 result = ((s32)regs.r[rm] * (s16)(regs.r[rs] >> 16)) >> 16;
    regs.r[rd] = result + op1;

    if (ADD_OVERFLOW(result, op1, regs.r[rd])) {
        SetConditionFlag(Q_FLAG, true);
    }

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

INSTRUCTION(ARM_SMULL) {
    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rdlo = (instruction >> 12) & 0xF;
    u8 rdhi = (instruction >> 16) & 0xF;
    s64 result = (s32)(regs.r[rm]);
    result *= (s32)(regs.r[rs]);
    regs.r[rdhi] = result >> 32;
    regs.r[rdlo] = result;
    
    regs.r[15] += 4;
}

INSTRUCTION(ARM_UMLAL) {
    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rdlo = (instruction >> 12) & 0xF;
    u8 rdhi = (instruction >> 16) & 0xF;
    u64 rdhilo = ((u64)regs.r[rdhi] << 32) | ((u64)regs.r[rdlo]);
    u64 result = (u64)regs.r[rm] * (u64)regs.r[rs] + rdhilo;
    
    regs.r[rdlo] = result & 0xFFFFFFFF;

    regs.r[rdhi] = result >> 32;

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