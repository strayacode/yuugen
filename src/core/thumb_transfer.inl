INSTRUCTION(THUMB_PUSH_LR) {
    // - 4 from r13 as lr is always going to be pushed in this instruction
    u32 address = regs.r[13];

    for (int i = 0; i < 8; i++) {
        if (instruction & (1 << i)) {
            address -= 4;
        }
    }

    address -= 4;

    // set r13
    regs.r[13] = address;

    for (int i = 0; i < 8; i++) {
        if (instruction & (1 << i)) {
            WriteWord(address, regs.r[i]);
            address += 4;
        }
    }

    // write lr to stack too
    WriteWord(address, regs.r[14]);

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_PUSH) {
    u32 address = regs.r[13];

    for (int i = 0; i < 8; i++) {
        if (instruction & (1 << i)) {
            address -= 4;
        }
    }

    // set r13
    regs.r[13] = address;

    for (int i = 0; i < 8; i++) {
        if (instruction & (1 << i)) {
            WriteWord(address, regs.r[i]);
            address += 4;
        }
    }

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_LDMIA_REG) {
    u8 rn = (instruction >> 8) & 0x7;
    u32 address = regs.r[rn];
    
    for (int i = 0; i < 8; i++) {
        if (instruction & (1 << i)) {
            regs.r[i] = ReadWord(address);
            address += 4;
        }
    }

    // if (instruction & (1 << 15)) {
    //     log_fatal("handle");
    // }

    // if rn is in rlist:
    // if arm9 writeback if rn is the only register or not the last register in rlist
    // if arm7 then no writeback if rn in rlist
    
    if (!(instruction & (1 << rn)) ||(arch == ARMv5 && ((instruction & 0xFF) == (1 << rn)) || !(((instruction & 0xFF) >> rn) == 1))) {
        regs.r[rn] = address;
    }

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_POP_PC) {
    u32 address = regs.r[13];

    for (int i = 0; i < 8; i++) {
        if (instruction & (1 << i)) {
            regs.r[i] = ReadWord(address);
            address += 4;
        }
    }

    // now handle r15 stuff
    regs.r[15] = ReadWord(address);
    
    address += 4;
    
    // writeback to r13
    regs.r[13] = address;

    // now to handle mode switch and stuff
    // if cpu is armv4 or bit 0 of r15 is 1 stay in thumb state
    if ((arch == ARMv4) || (regs.r[15] & 0x1)) {
        // halfword align r15 and flush pipeline
        regs.r[15] &= ~1;
        ThumbFlushPipeline();
    } else {
        // clear bit 5 of cpsr to switch to arm state
        regs.cpsr &= ~(1 << 5);
        regs.r[15] &= ~3;
        ARMFlushPipeline();
    }
}

INSTRUCTION(THUMB_POP) {
    u32 address = regs.r[13];

    for (int i = 0; i < 8; i++) {
        if (instruction & (1 << i)) {
            regs.r[i] = ReadWord(address);
            address += 4;
        }
    }

    
    // writeback to r13
    regs.r[13] = address;

    regs.r[15] += 2;
}

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

INSTRUCTION(THUMB_LDRH_REG) {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;
    u8 rm = (instruction >> 6) & 0x7;

    u32 address = regs.r[rn] + regs.r[rm];

    regs.r[rd] = ReadHalfword(address);

    if (rd == 15) {
        log_fatal("handle");
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

INSTRUCTION(THUMB_LDRSH_REG) {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;
    u8 rm = (instruction >> 6) & 0x7;

    u32 address = regs.r[rn] + regs.r[rm];

    regs.r[rd] = (s16)ReadHalfword(address);

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

INSTRUCTION(THUMB_STRH_REG) {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;
    u8 rm = (instruction >> 6) & 0x7;

    u32 address = regs.r[rn] + regs.r[rm];

    WriteHalfword(address, regs.r[rd]);

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

INSTRUCTION(THUMB_STRH_IMM5) {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;

    u32 immediate = (instruction >> 6) & 0x1F;
    u32 address = regs.r[rn] + (immediate << 1);
    WriteHalfword(address, regs.r[rd]);

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

INSTRUCTION(THUMB_LDRH_IMM5) {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;
    u32 immediate = (instruction >> 6) & 0x1F;
    u32 address = regs.r[rn] + (immediate << 1);

    regs.r[rd] = ReadHalfword(address);

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