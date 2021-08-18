#pragma once

void ThumbAddSubtract() {
    u8 rn = (instruction >> 6) & 0x7;
    u8 rs = (instruction >> 3) & 0x7;
    u8 rd = instruction & 0x7;

    bool immediate = instruction & (1 << 10);
    bool sub = instruction & (1 << 9);

    u32 operand = immediate ? rn : regs.r[rn];

    if (sub) {
        regs.r[rd] = regs.r[rs] - operand;
        SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
        SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
        SetConditionFlag(C_FLAG, SUB_CARRY(regs.r[rs], operand));
        SetConditionFlag(V_FLAG, SUB_OVERFLOW(regs.r[rs], operand, regs.r[rd]));
    } else {
        regs.r[rd] = regs.r[rs] + operand;
        SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
        SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
        SetConditionFlag(C_FLAG, ADD_CARRY(regs.r[rs], operand));
        SetConditionFlag(V_FLAG, ADD_OVERFLOW(regs.r[rs], operand, regs.r[rd]));
    }

    regs.r[15] += 2;
}

void ThumbShiftImmediate() {
    u8 rd = instruction & 0x7;
    u8 rs = (instruction >> 3) & 0x7;

    u8 shift_amount = (instruction >> 6) & 0x1F;
    u8 shift_type = (instruction >> 11) & 0x3;

    u8 carry = GetConditionFlag(C_FLAG);

    switch (shift_type) {
    case 0x0:
        if (shift_amount != 0) {
            carry = (regs.r[rs] >> (32 - shift_amount)) & 0x1;
        }

        regs.r[rd] = regs.r[rs] << shift_amount;
        break;
    case 0x1:
        if (shift_amount == 0) {
            carry = regs.r[rs] >> 31;
            regs.r[rd] = 0;
        } else {
            carry = (regs.r[rs] >> (shift_amount - 1)) & 0x1;
            regs.r[rd] = regs.r[rs] >> shift_amount;
        } 
        break;
    case 0x2: {
        u32 msb = regs.r[rs] >> 31;

        if (shift_amount == 0) {
            carry = regs.r[rd] >> 31;
            regs.r[rd] = 0xFFFFFFFF * msb;
        } else {
            carry = (regs.r[rs] >> (shift_amount - 1)) & 0x1;
            regs.r[rd] = (regs.r[rs] >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
        }
        break;
    }
    case 0x3:
        log_fatal("[Interpreter] incorrect opcode %08x", instruction);
    }

    SetConditionFlag(C_FLAG, carry);
    SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
    SetConditionFlag(N_FLAG, regs.r[rd] >> 31);

    regs.r[15] += 2;
}

void ThumbALUImmediate() {
    u8 immediate = instruction & 0xFF;
    u8 rd = (instruction >> 8) & 0x7;
    u8 opcode = (instruction >> 11) & 0x3;

    u32 result = 0;

    switch (opcode) {
    case 0x0:
        result = immediate;
        regs.r[rd] = immediate;
        break;
    case 0x1:
        result = regs.r[rd] - immediate;
        SetConditionFlag(C_FLAG, SUB_CARRY(regs.r[rd], immediate));
        SetConditionFlag(V_FLAG, SUB_OVERFLOW(regs.r[rd], immediate, result));
        break;
    case 0x2:
        result = regs.r[rd] + immediate;
        SetConditionFlag(C_FLAG, ADD_CARRY(regs.r[rd], immediate));
        SetConditionFlag(V_FLAG, ADD_OVERFLOW(regs.r[rd], immediate, result));
        regs.r[rd] += immediate;
        break;
    case 0x3:
        result = regs.r[rd] - immediate;
        SetConditionFlag(C_FLAG, SUB_CARRY(regs.r[rd], immediate));
        SetConditionFlag(V_FLAG, SUB_OVERFLOW(regs.r[rd], immediate, result));
        regs.r[rd] -= immediate;
        break;
    default:
        log_fatal("handle opcode %d", opcode);
    }

    SetConditionFlag(N_FLAG, result >> 31);
    SetConditionFlag(Z_FLAG, result == 0);

    regs.r[15] += 2;
}

void ThumbDataProcessingRegister() {
    u8 rd = instruction & 0x7;
    u8 rs = (instruction >> 3) & 0x7;

    u8 opcode = (instruction >> 6) & 0xF;

    u8 carry = GetConditionFlag(C_FLAG);

    switch (opcode) {
    case 0x0:
        regs.r[rd] = AND(regs.r[rd], regs.r[rs], true);
        break;
    case 0x1:
        regs.r[rd] = EOR(regs.r[rd], regs.r[rs], true);
        break;
    case 0x2:
        regs.r[rd] = LSL(regs.r[rd], regs.r[rs] & 0xFF, carry);
        SetConditionFlag(C_FLAG, carry);
        SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
        SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
        break;
    case 0x3:
        regs.r[rd] = LSR(regs.r[rd], regs.r[rs] & 0xFF, carry, false);
        SetConditionFlag(C_FLAG, carry);
        SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
        SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
        break;
    case 0x5:
        regs.r[rd] = ADC(regs.r[rd], regs.r[rs], true);
        break;
    case 0x6:
        regs.r[rd] = SBC(regs.r[rd], regs.r[rs], true);
        break;
    case 0x7:
        regs.r[rd] = ROR(regs.r[rd], regs.r[rs] & 0xFF, carry, false);
        SetConditionFlag(C_FLAG, carry);
        SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
        SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
        break;
    case 0x8:
        TST(regs.r[rd], regs.r[rs]);
        break;
    case 0x9:
        regs.r[rd] = SUB(0, regs.r[rs], true);
        break;
    case 0xA:
        CMP(regs.r[rd], regs.r[rs]);
        break;
    case 0xC:
        regs.r[rd] = ORR(regs.r[rd], regs.r[rs], true);
        break;
    case 0xD:
        regs.r[rd] *= regs.r[rs];

        SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
        SetConditionFlag(Z_FLAG, regs.r[rd] == 0);

        if (arch == CPUArch::ARMv4) {
            SetConditionFlag(C_FLAG, false);
        }
        break;
    case 0xE:
        regs.r[rd] = BIC(regs.r[rd], regs.r[rs], true);
        break;
    case 0xF:
        regs.r[rd] = MVN(regs.r[rd], regs.r[rs]);
        break;
    default:
        log_fatal("handle opcode %02x %08x", opcode, instruction);
    }

    regs.r[15] += 2;
}

void ThumbSpecialDataProcesing() {
    u8 rd = ((instruction & (1 << 7)) >> 4) | (instruction & 0x7);
    u8 rs = (instruction >> 3) & 0xF;

    u8 opcode = (instruction >> 8) & 0x3;

    switch (opcode) {
    case 0x0:
        regs.r[rd] += regs.r[rs];
        regs.r[15] += 2;
        break;
    case 0x1:
        CMP(regs.r[rd], regs.r[rs]);
        regs.r[15] += 2;
        break;
    case 0x2:
        regs.r[rd] = regs.r[rs];
        regs.r[15] += 2;
        break;
    default:
        log_fatal("handle opcode %d", opcode);
    }

    if (rd == 15) {
        log_fatal("handle");
    }
}

void ThumbAdjustStackPointer() {
    u32 immediate = (instruction & 0x7F) << 2;

    // need to check bit 7 to check if we subtract or add from sp
    regs.r[13] = regs.r[13] + ((instruction & (1 << 7)) ? - immediate : immediate);

    regs.r[15] += 2;
}

void ThumbAddSPPC() {
    u32 immediate = (instruction & 0xFF) << 2;
    u8 rd = (instruction >> 8) & 0x7;
    bool sp = instruction & (1 << 11);

    if (sp) {
        regs.r[rd] = regs.r[13] + immediate;
    } else {
        regs.r[rd] = (regs.r[15] & ~0x2) + immediate;
    }

    regs.r[15] += 2;
}