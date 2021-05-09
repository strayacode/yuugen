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

    // if rn is in rlist:
    // if arm9 writeback if rn is the only register or not the last register in rlist
    // if arm7 then no writeback if rn in rlist
    if (arch == ARMv5) {
        if (((instruction & 0xFF) == (unsigned int)(1 << rn)) || !(((instruction & 0xFF) >> rn) == 1)) {
            regs.r[rn] = address;
        }
    } else {
        if (!(instruction & (unsigned int)(1 << rn))) {
            regs.r[rn] = address;
        }
    }

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_STMIA_REG) {
    u8 rn = (instruction >> 8) & 0x7;
    u32 address = regs.r[rn];

    for (int i = 0; i < 8; i++) {
        if (instruction & (1 << i)) {
            WriteWord(address, regs.r[i]);
            address += 4;
        }
    }

    // writeback to rn
    regs.r[rn] = address;

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