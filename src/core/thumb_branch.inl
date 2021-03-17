INSTRUCTION(THUMB_B) {
    u32 offset = ((instruction & (1 << 10)) ? 0xFFFFF000 : 0) | ((instruction & 0x7FF) << 1);
    regs.r[15] += offset;
    ThumbFlushPipeline();
}

INSTRUCTION(THUMB_BEQ) {
    // only branch if z flag is set
    if (GetConditionFlag(Z_FLAG)) {
        u32 offset = ((instruction & (1 << 7)) ? 0xFFFFFE00 : 0) | ((instruction & 0xFF) << 1);
        regs.r[15] += offset;
        ThumbFlushPipeline();
    } else {
        regs.r[15] += 2;
    }
}

INSTRUCTION(THUMB_BLE) {
    // only branch if condition is true
    if (GetConditionFlag(Z_FLAG) || (GetConditionFlag(N_FLAG) != GetConditionFlag(V_FLAG))) {
        u32 offset = ((instruction & (1 << 7)) ? 0xFFFFFE00 : 0) | ((instruction & 0xFF) << 1);
        regs.r[15] += offset;
        ThumbFlushPipeline();
    } else {
        regs.r[15] += 2;
    }
}

INSTRUCTION(THUMB_BLS) {
    // only branch if condition is true
    if (!GetConditionFlag(C_FLAG) || GetConditionFlag(Z_FLAG)) {
        u32 offset = ((instruction & (1 << 7)) ? 0xFFFFFE00 : 0) | ((instruction & 0xFF) << 1);
        regs.r[15] += offset;
        ThumbFlushPipeline();
    } else {
        regs.r[15] += 2;
    }
}

INSTRUCTION(THUMB_BGE) {
    // only branch if condition is true
    if (GetConditionFlag(N_FLAG) == GetConditionFlag(V_FLAG)) {
        u32 offset = ((instruction & (1 << 7)) ? 0xFFFFFE00 : 0) | ((instruction & 0xFF) << 1);
        regs.r[15] += offset;
        ThumbFlushPipeline();
    } else {
        regs.r[15] += 2;
    }
}

INSTRUCTION(THUMB_BMI) {
    // only branch if condition is true
    if (GetConditionFlag(N_FLAG)) {
        u32 offset = ((instruction & (1 << 7)) ? 0xFFFFFE00 : 0) | ((instruction & 0xFF) << 1);
        regs.r[15] += offset;
        ThumbFlushPipeline();
    } else {
        regs.r[15] += 2;
    }
}

INSTRUCTION(THUMB_BCC) {
    // only branch if c flag is cleared
    if (!GetConditionFlag(C_FLAG)) {
        u32 offset = ((instruction & (1 << 7)) ? 0xFFFFFE00 : 0) | ((instruction & 0xFF) << 1);
        regs.r[15] += offset;
        ThumbFlushPipeline();
    } else {
        regs.r[15] += 2;
    }
}

INSTRUCTION(THUMB_BHI) {
    // only branch if condition is true
    if (GetConditionFlag(C_FLAG) && !GetConditionFlag(Z_FLAG)) {
        u32 offset = ((instruction & (1 << 7)) ? 0xFFFFFE00 : 0) | ((instruction & 0xFF) << 1);
        regs.r[15] += offset;
        ThumbFlushPipeline();
    } else {
        regs.r[15] += 2;
    }
}

INSTRUCTION(THUMB_BGT) {
    // only branch if condition is true
    if ((!GetConditionFlag(Z_FLAG)) && (GetConditionFlag(N_FLAG) == GetConditionFlag(V_FLAG))) {
        u32 offset = ((instruction & (1 << 7)) ? 0xFFFFFE00 : 0) | ((instruction & 0xFF) << 1);
        regs.r[15] += offset;
        ThumbFlushPipeline();
    } else {
        regs.r[15] += 2;
    }
}

INSTRUCTION(THUMB_BPL) {
    // only branch if n flag cleared
    if (!GetConditionFlag(N_FLAG)) {
        u32 offset = ((instruction & (1 << 7)) ? 0xFFFFFE00 : 0) | ((instruction & 0xFF) << 1);
        regs.r[15] += offset;
        ThumbFlushPipeline();
    } else {
        regs.r[15] += 2;
    }
}

INSTRUCTION(THUMB_BNE) {
    // only branch if zero flag cleared
    if (!GetConditionFlag(Z_FLAG)) {
        u32 offset = ((instruction & (1 << 7)) ? 0xFFFFFE00 : 0) | ((instruction & 0xFF) << 1);
        regs.r[15] += offset;
        ThumbFlushPipeline();
    } else {
        regs.r[15] += 2;
    }
}

INSTRUCTION(THUMB_BLT) {
    // only branch if n not equal to v
    if (GetConditionFlag(N_FLAG) != GetConditionFlag(V_FLAG)) {
        u32 offset = ((instruction & (1 << 7)) ? 0xFFFFFE00 : 0) | ((instruction & 0xFF) << 1);
        regs.r[15] += offset;
        ThumbFlushPipeline();
    } else {
        regs.r[15] += 2;
    }
}

INSTRUCTION(THUMB_BCS) {
    // only branch if c flag is set
    if (GetConditionFlag(C_FLAG)) {
        u32 offset = ((instruction & (1 << 7)) ? 0xFFFFFE00 : 0) | ((instruction & 0xFF) << 1);
        regs.r[15] += offset;
        ThumbFlushPipeline();
    } else {
        regs.r[15] += 2;
    }
}

INSTRUCTION(THUMB_BX_REG) {
    u8 rm = (instruction >> 3) & 0xF;
    if (regs.r[rm] & 0x1) {
        // just load rm into r15 normally in thumb
        regs.r[15] = regs.r[rm] & ~1;
        ThumbFlushPipeline();
    } else {
        // switch to arm state
        // clear bit 5 in cpsr
        regs.cpsr &= ~(1 << 5);
        regs.r[15] = regs.r[rm] & ~3;
        ARMFlushPipeline();
    }
}

INSTRUCTION(THUMB_BL_SETUP) {
    u32 immediate = ((instruction & (1 << 10)) ? 0xFFFFF000 : 0) | ((instruction & 0x7FF) << 1);

    regs.r[14] = regs.r[15] + (immediate << 11);

    regs.r[15] += 2;
}

INSTRUCTION(THUMB_BL_OFFSET) {
    u32 offset = (instruction & 0x7FF) << 1;
    u32 next_instruction_address = regs.r[15] - 1;
    regs.r[15] = (regs.r[14] + offset) & ~1;
    regs.r[14] = next_instruction_address;
    ThumbFlushPipeline();
}