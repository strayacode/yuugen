#pragma once

void ThumbAddSubtract() {
    u8 opcode = (instruction >> 9) & 0x3;
    u8 operand = (instruction >> 6) & 0x7;
    u8 rs = (instruction >> 3) & 0x7;
    u8 rd = instruction & 0x7;

    switch (opcode) {
    case 0:
        regs.r[rd] = regs.r[rs] + regs.r[operand];
        SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
        SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
        SetConditionFlag(C_FLAG, ADD_CARRY(regs.r[rs], regs.r[operand]));
        SetConditionFlag(V_FLAG, ADD_OVERFLOW(regs.r[rs], regs.r[operand], regs.r[rd]));
        break;
    case 1:
        regs.r[rd] = regs.r[rs] - regs.r[operand];
        SetConditionFlag(N_FLAG, regs.r[rd] >> 31);
        SetConditionFlag(Z_FLAG, regs.r[rd] == 0);
        SetConditionFlag(C_FLAG, SUB_CARRY(regs.r[rs], regs.r[operand]));
        SetConditionFlag(V_FLAG, SUB_OVERFLOW(regs.r[rs], regs.r[operand], regs.r[rd]));
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


    // TODO: actually make correct later
    switch (shift_type) {
    case 0x0:
        if (shift_amount != 0) {
            regs.r[rd] = regs.r[rs] << shift_amount;
        }
        break;
    case 0x1:
        regs.r[rd] = regs.r[rs] >> shift_amount;
        break;
    case 0x2: {
        u8 msb = regs.r[rs] >> 31;
        regs.r[rd] = (regs.r[rs] >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
        break;
    }
    case 0x3:
        regs.r[rd] = rotate_right(regs.r[rs], shift_amount);
        break;
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
    case 0x2: {
        result = regs.r[rd] + immediate;
        SetConditionFlag(C_FLAG, ADD_CARRY(regs.r[rd], immediate));
        SetConditionFlag(V_FLAG, ADD_OVERFLOW(regs.r[rd], immediate, result));
        regs.r[rd] += immediate;
        break;
    }
    case 0x3: {
        result = regs.r[rd] - immediate;
        SetConditionFlag(C_FLAG, SUB_CARRY(regs.r[rd], immediate));
        SetConditionFlag(V_FLAG, SUB_OVERFLOW(regs.r[rd], immediate, result));
        regs.r[rd] -= immediate;
        break;
    }
    default:
        log_fatal("handle opcode %d", opcode);
    }

    SetConditionFlag(N_FLAG, result >> 31);
    SetConditionFlag(Z_FLAG, result == 0);

    regs.r[15] += 2;
}

void ThumbDataProcessingRegister() {
    log_fatal("handle");
}

void ThumbSpecialDataProcesing() {
    log_fatal("handle");
}