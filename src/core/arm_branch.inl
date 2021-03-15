INSTRUCTION(ARM_B) {
    u32 offset = ((instruction & (1 << 23)) ? 0xFF000000 : 0) | ((instruction & 0xFFFFFF) << 2);
    // r15 is at instruction address + 8
    regs.r[15] += offset;

    ARMFlushPipeline();
}

INSTRUCTION(ARM_BL) {
    u32 offset = ((instruction & (1 << 23)) ? 0xFF000000 : 0) | ((instruction & 0xFFFFFF) << 2);
    
    // store the address of the instruction after the current instruction in the link register
    regs.r[14] = regs.r[15] - 4;

    // r15 is at instruction address + 8
    regs.r[15] += offset;

    ARMFlushPipeline();
}

INSTRUCTION(ARM_BX) {
    u8 rm = instruction & 0xF;
    if (regs.r[rm] & 0x1) {
        // set bit 5 of cpsr to switch to thumb state
        regs.cpsr |= (1 << 5);
        regs.r[15] = regs.r[rm] & ~1;
        ThumbFlushPipeline();
    } else {
        // no need to clear bit 5 of cpsr as we are already in arm mode
        regs.r[15] = regs.r[rm] & ~3;
        ARMFlushPipeline();
    }
}

INSTRUCTION(ARM_BLX_REG) {
    // ARMv5 exclusive instruction
    if (arch == ARMv4) {
        return;
    }

    // store the address of the instruction after the blx in the link register
    regs.r[14] = regs.r[15] - 4;

    u8 rm = instruction & 0xF;
    if (regs.r[rm] & 0x1) {
        // set bit 5 of cpsr to switch to thumb state
        regs.cpsr |= (1 << 5);
        regs.r[15] = regs.r[rm] & ~1;
        ThumbFlushPipeline();
    } else {
        // no need to clear bit 5 of cpsr as we are already in arm mode
        regs.r[15] = regs.r[rm] & ~3;
        ARMFlushPipeline();
    }
}

INSTRUCTION(ARM_BLX) {
    // arm9 specific instruction
    if (arch == ARMv4) {
        return;
    }

    // store address of instruction after blx instruction into lr
    regs.r[14] = regs.r[15] - 4;
    // set the t flag to 1 (switch to thumb mode)
    regs.cpsr |= (1 << 5);

    u32 offset = (((instruction & (1 << 23)) ? 0xFF000000: 0) | ((instruction & 0xFFFFFF) << 2)) + ((instruction & (1 << 24)) << 1);
    regs.r[15] += offset;
    // since we switched to thumb mode do a 16 bit pipeline flush
    ThumbFlushPipeline();
}
