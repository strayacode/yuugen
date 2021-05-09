INSTRUCTION(THUMB_LDR_PC) {
    u32 immediate = (instruction & 0xFF) << 2;
    u8 rd = (instruction >> 8) & 0x7;
    // in this instruction pc is word aligned (pc & 0xFFFFFFFC)
    u32 address = (regs.r[15] & ~0x3) + immediate;
    regs.r[rd] = ReadWord(address);
    // log_fatal("address is 0x%04x", address);
    if (address & 0x3) {
        u8 shift_amount = (address & 0x3) << 3;
        regs.r[rd] = (regs.r[rd] >> shift_amount) | (regs.r[rd] << (32 - shift_amount));
    }

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_STR_SP) {
    u8 rd = (instruction >> 8) & 0x7;
    u32 immediate = instruction & 0xFF;
    u32 address = regs.r[13] + (immediate << 2);
    WriteWord(address, regs.r[rd]);

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_LDR_SP) {
    u32 immediate = instruction & 0xFF;
    u8 rd = (instruction >> 8) & 0x7;
    u32 address = regs.r[13] + (immediate << 2);
    regs.r[rd] = ReadWord(address);

    if (address & 0x3) {
        log_fatal("handle");
        int shift_amount = (address & 0x3) * 8;
        regs.r[rd] = (regs.r[rd] << (32 - shift_amount)) | (regs.r[rd] >> shift_amount);
    } 

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_LDR_REG) {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;
    u8 rm = (instruction >> 6) & 0x7;

    u32 address = regs.r[rn] + regs.r[rm];

    regs.r[rd] = ReadWord(address);

    if (rd == 15) {
        log_fatal("handle");
    }

    // deal with misaligned reads
    if (address & 0x3) {
        int shift_amount = (address & 0x3) * 8;
        regs.r[rd] = (regs.r[rd] << (32 - shift_amount)) | (regs.r[rd] >> shift_amount);
    }

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_LDRB_REG) {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;
    u8 rm = (instruction >> 6) & 0x7;

    u32 address = regs.r[rn] + regs.r[rm];

    regs.r[rd] = ReadByte(address);

    if (rd == 15) {
        log_fatal("handle");
    }

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_STR_REG) {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;
    u8 rm = (instruction >> 6) & 0x7;

    u32 address = regs.r[rn] + regs.r[rm];

    WriteWord(address, regs.r[rd]);

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_STRB_REG) {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;
    u8 rm = (instruction >> 6) & 0x7;

    u32 address = regs.r[rn] + regs.r[rm];

    WriteByte(address, regs.r[rd]);

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_STRB_IMM5) {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;

    u32 immediate = (instruction >> 6) & 0x1F;
    u32 address = regs.r[rn] + immediate;
    WriteByte(address, regs.r[rd]);

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_STR_IMM5) {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;
    u32 immediate = (instruction >> 6) & 0x1F;
    u32 address = regs.r[rn] + (immediate << 2);

    WriteWord(address, regs.r[rd]);

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_LDR_IMM5) {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;
    u32 immediate = (instruction >> 6) & 0x1F;
    u32 address = regs.r[rn] + (immediate << 2);

    regs.r[rd] = ReadWord(address);
    
    // deal with misaligned reads
    if (address & 0x3) {
        int shift_amount = (address & 0x3) * 8;
        regs.r[rd] = (regs.r[rd] << (32 - shift_amount)) | (regs.r[rd] >> shift_amount);
    }

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_LDRB_IMM5) {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;
    u32 immediate = (instruction >> 6) & 0x1F;
    u32 address = regs.r[rn] + immediate;
    regs.r[rd] = ReadByte(address);
    // // deal with misaligned reads
    // if (address & 0x3) {
    //     int shift_amount = (address & 0x3) * 8;
    //     regs.r[rd] = (regs.r[rd] << (32 - shift_amount)) | (regs.r[rd] >> shift_amount);
    // }

    regs.r[15] += 2;
}