#pragma once

void ThumbAddSubtract() {
    u8 opcode = (instruction >> 9) & 0x3;
    u8 rn = (instruction >> 6) & 0x7;
    u8 rs = (instruction >> 3) & 0x7;
    u8 rd = instruction & 0x7;

    bool immediate = instruction & (1 << 10);

    u32 operand = immediate ? rn : regs.r[rn];

    switch (opcode) {
    case 0x0:
        regs.r[rd] = regs.r[rs] + operand;
        SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
        SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
        SetConditionFlag(C_FLAG, ADD_CARRY(regs.r[rs], operand));
        SetConditionFlag(V_FLAG, ADD_OVERFLOW(regs.r[rs], operand, regs.r[rd]));
        break;
    case 0x1:
        regs.r[rd] = regs.r[rs] - operand;
        SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
        SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
        SetConditionFlag(C_FLAG, SUB_CARRY(regs.r[rs], operand));
        SetConditionFlag(V_FLAG, SUB_OVERFLOW(regs.r[rs], operand, regs.r[rd]));
        break;
    default:
        log_fatal("handle opcode %d", opcode);
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
            regs.r[rd] = regs.r[rs] << shift_amount;
            carry = (regs.r[rs] >> (32 - shift_amount)) & 0x1;
        }
        break;
    case 0x1:
        if (shift_amount == 0) {
            carry = regs.r[rd] >> 31;
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
            regs.r[rd] = 0;
        } else {
            carry = (regs.r[rs] >> (shift_amount - 1)) & 0x1;
            regs.r[rd] = (regs.r[rs] >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
        }
        break;
    }
    case 0x3:
        log_fatal("[Interpreter] incorrect opcode");
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

    switch (opcode) {
    case 0xC:
        regs.r[rd] |= regs.r[rs];
        break;
    default:
        log_fatal("handle opcode %02x", opcode);
    }

    SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
    SetConditionFlag(Z_FLAG, regs.r[rd] == 0);

    regs.r[15] += 2;
}

void ThumbSpecialDataProcesing() {
    u8 rd = ((instruction & (1 << 7)) >> 4) | (instruction & 0x7);
    u8 rs = (instruction >> 3) & 0xF;

    u8 opcode = (instruction >> 8) & 0x3;

    switch (opcode) {
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
    log_fatal("handle")
}

void ThumbAddSPPC() {
    log_fatal("handle");
}