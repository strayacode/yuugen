INSTRUCTION(ARM_MSR_CPSR_REG) {
    u8 rm = instruction & 0xF;

    if (instruction & (1 << 16) && PrivilegedMode()) {
        SwitchMode(regs.r[rm] & 0x1F);
    }

    u32 mask = 0;
    if (instruction & (1 << 16)) {
        mask |= 0x000000FF;
    }
    if (instruction & (1 << 17)) {
        mask |= 0x0000FF00;
    }
    if (instruction & (1 << 18)) {
        mask |= 0x00FF0000;
    }
    if (instruction & (1 << 19)) {
        mask |= 0xFF000000;
    }

    u32 value = regs.r[rm] & mask;

    regs.cpsr = (regs.cpsr & ~mask) | value;

    regs.r[15] += 4;
}

INSTRUCTION(ARM_MSR_CPSR_IMM) {
    u32 immediate = instruction & 0xFF;
    u8 rotate_amount = ((instruction >> 8) & 0xF) << 1;

    u32 result = (immediate >> rotate_amount) | (immediate << (32 - rotate_amount));

    if (instruction & (1 << 16) && PrivilegedMode()) {
        SwitchMode(result & 0x1F);
    }

    u32 mask = 0;
    if (instruction & (1 << 16)) {
        mask |= 0x000000FF;
    }
    if (instruction & (1 << 17)) {
        mask |= 0x0000FF00;
    }
    if (instruction & (1 << 18)) {
        mask |= 0x00FF0000;
    }
    if (instruction & (1 << 19)) {
        mask |= 0xFF000000;
    }

    u32 value = result & mask;

    regs.cpsr = (regs.cpsr & ~mask) | value;
    
    regs.r[15] += 4;
}

INSTRUCTION(ARM_MSR_SPSR_REG) {
    u8 rm = instruction & 0xF;

    u32 mask = 0;
    if (instruction & (1 << 16)) {
        mask |= 0x000000FF;
    }
    if (instruction & (1 << 17)) {
        mask |= 0x0000FF00;
    }
    if (instruction & (1 << 18)) {
        mask |= 0x00FF0000;
    }
    if (instruction & (1 << 19)) {
        mask |= 0xFF000000;
    }

    u32 value = regs.r[rm] & mask;

    if (HasSPSR()) {
        SetCurrentSPSR((GetCurrentSPSR() & ~mask) | value);
    }
    
    regs.r[15] += 4;
}

INSTRUCTION(ARM_MRS_CPSR) {
    u8 rd = (instruction >> 12) & 0xF;

    regs.r[rd] = regs.cpsr;

    regs.r[15] += 4;
}

INSTRUCTION(ARM_MRS_SPSR) {
    u8 rd = (instruction >> 12) & 0xF;

    regs.r[rd] = GetCurrentSPSR();

    regs.r[15] += 4;
}