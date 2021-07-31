#pragma once

void ARMCoprocessorRegisterTransfer() {
    log_fatal("handle");
}

void ARMPSRTransfer() {
    log_fatal("handle");
}

void ARMSingleDataTransfer() {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;

    u8 load = (instruction >> 20) & 0x1;
    u8 writeback = (instruction >> 21) & 0x1;
    u8 byte = (instruction >> 22) & 0x1;
    u8 up = (instruction >> 23) & 0x1;
    u8 pre = (instruction >> 24) & 0x1;
    u8 shifted_register = (instruction >> 25) & 0x1;

    u32 op2 = 0;
    u32 address = 0;

    if (shifted_register) {
        op2 = ARMGetShiftedRegisterSingleDataTransfer(instruction);
    } else {
        op2 = instruction & 0xFFF;
    }

    if (!up) {
        op2 *= -1;
    }

    if (pre) {
        address = regs.r[rn] + op2;

        if (writeback) {
            regs.r[rn] += op2;
        }
    } else {
        address = regs.r[rn];
    }

    if (load) {
        if (byte) {
            regs.r[rd] = ReadByte(address);
        } else {
            regs.r[rd] = ReadWord(address);
        }

        // TODO: align loads
    } else {
        if (byte) {
            WriteByte(address, regs.r[rd]);
        } else {
            WriteWord(address, regs.r[rd]);
        }
    }

    if (rd == 15) {
        log_fatal("handle");
    }

    regs.r[15] += 4;
}

auto ARMGetShiftedRegisterSingleDataTransfer(u32 instruction) -> u32 {
    u8 shift_type = (instruction >> 5) & 0x3;
    u8 shift_amount = (instruction >> 7) & 0x1F;

    u8 rm = instruction & 0xF;

    u32 op2 = 0;

    switch (shift_type) {
    case 0x0:
        // LSL
        op2 = regs.r[rm] << shift_amount;
        break;
    case 0x1:
        // LSR
        if (shift_amount != 0) {
            op2 = regs.r[rm] >> shift_amount;
        }
        break;
    case 0x2: {
        // ASR
        u8 msb = regs.r[rm] >> 31;

        if (shift_amount == 0) {
            op2 = 0xFFFFFFFF * msb;
        } else {
            op2 = (regs.r[rm] >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
        }
        break;
    }
    case 0x3:
        // ROR
        if (shift_amount == 0) {
            // rotate right extended
            op2 = (GetConditionFlag(C_FLAG) << 31) | (regs.r[rm] >> 1);
        } else {
            // rotate right
            op2 = rotate_right(regs.r[rm], shift_amount);
        }
        break;
    }

    return op2;
}

void ARMHalfwordDataTransfer() {
    u8 rm = instruction & 0xF;
    u8 opcode = (instruction >> 5) & 0x3;
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    u8 load = (instruction >> 20) & 0x1;
    u8 writeback = (instruction >> 21) & 0x1;
    u8 immediate = (instruction >> 22) & 0x1;
    u8 up = (instruction >> 23) & 0x1;
    u8 pre = (instruction >> 24) & 0x1;

    u32 op2 = 0;
    u32 address = regs.r[rn];

    if (immediate) {
        op2 = ((instruction >> 4) & 0xF0) | (instruction & 0xF);
    } else {
        op2 = regs.r[rm];
    }

    if (!up) {
        op2 *= -1;
    }

    if (pre) {
        address += op2;
    }

    switch (opcode) {
    case 0x1:
        if (load) {
            regs.r[rd] = ReadHalf(address);
        } else {
            WriteHalf(address, regs.r[rd]);
        }
        break;
    default:
        log_fatal("handle opcode %d", opcode);
    }

    if (writeback) {
        regs.r[rn] += op2;
    }

    if (rd == 15) {
        log_fatal("handle");
    }

    regs.r[15] += 4;
}

void ARMBlockDataTransfer() {
    log_fatal("handle ldm/stm");
}