INSTRUCTION(THUMB_LDRH_REG) {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;
    u8 rm = (instruction >> 6) & 0x7;

    u32 address = regs.r[rn] + regs.r[rm];

    regs.r[rd] = ReadHalf(address);

    if (rd == 15) {
        log_fatal("handle");
    }

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_LDRSB_REG) {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;
    u8 rm = (instruction >> 6) & 0x7;

    u32 address = regs.r[rn] + regs.r[rm];

    regs.r[rd] = (s8)ReadByte(address);

    if (rd == 15) {
        log_fatal("handle");
    }

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_LDRSH_REG) {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;
    u8 rm = (instruction >> 6) & 0x7;

    u32 address = regs.r[rn] + regs.r[rm];

    regs.r[rd] = (s16)ReadHalf(address);

    if (rd == 15) {
        log_fatal("handle");
    }

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_STRH_REG) {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;
    u8 rm = (instruction >> 6) & 0x7;

    u32 address = regs.r[rn] + regs.r[rm];

    WriteHalf(address, regs.r[rd]);

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_STRH_IMM5) {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;

    u32 immediate = (instruction >> 6) & 0x1F;
    u32 address = regs.r[rn] + (immediate << 1);
    WriteHalf(address, regs.r[rd]);

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_LDRH_IMM5) {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;
    u32 immediate = (instruction >> 6) & 0x1F;
    u32 address = regs.r[rn] + (immediate << 1);

    regs.r[rd] = ReadHalf(address);

    regs.r[15] += 2;
}