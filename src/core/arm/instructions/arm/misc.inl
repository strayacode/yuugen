u32 ARM_DATA_PROCESSING_IMM() {
    u32 immediate = instruction & 0xFF;
    u8 shift_amount = ((instruction >> 8) & 0xF) << 1;

    u32 result = (immediate >> shift_amount) | (immediate << (32 - shift_amount));
    return result;
}

u32 ARM_DATA_PROCESSING_IMMS() {
    u32 immediate = instruction & 0xFF;
    u8 shift_amount = ((instruction >> 8) & 0xF) << 1;

    u32 result = (immediate >> shift_amount) | (immediate << (32 - shift_amount));
    
    if (shift_amount != 0) {
        SetConditionFlag(C_FLAG, result >> 31);
    }
    return result;
}

// shifts
u32 ARM_LOGICAL_SHIFT_LEFT_IMM() {
    u8 shift_amount = (instruction >> 7) & 0x1F;
    u8 rm = instruction & 0xF;
    return regs.r[rm] << shift_amount;
}

u32 ARM_LOGICAL_SHIFT_LEFT_IMMS() {
    u8 shift_amount = (instruction >> 7) & 0x1F;
    u8 rm = instruction & 0xF;

    if (shift_amount > 0) {
        // shifter_carry_out (c flag) = rm[32 - shift_amount]
        SetConditionFlag(C_FLAG, regs.r[rm] & (1 << (32 - shift_amount)));
    }

    return regs.r[rm] << shift_amount;
}

u32 ARM_LOGICAL_SHIFT_LEFT_REG() {
    u8 shift_amount = regs.r[(instruction >> 8) & 0xF] & 0xFF;
    u32 rm = regs.r[instruction & 0xF] + (((instruction & 0xF) == 15) ? 4 : 0);
    
    u32 result = 0;

    result = rm << shift_amount;

    return result;
}

u32 ARM_LOGICAL_SHIFT_LEFT_REGS() {
    u8 shift_amount = regs.r[(instruction >> 8) & 0xF] & 0xFF;
    u32 rm = regs.r[instruction & 0xF] + (((instruction & 0xF) == 15) ? 4 : 0);
    
    u32 result = 0;
    if (shift_amount == 0) {
        result = rm;
    } else if (shift_amount < 32) {
        result = rm << shift_amount;
        SetConditionFlag(C_FLAG, rm & (1 << (32 - shift_amount)));
    } else if (shift_amount == 32) {
        SetConditionFlag(C_FLAG, rm & 0x1);
    } else {
        // shift amount > 32
        SetConditionFlag(C_FLAG, false);
    }

    return result;
}

u32 ARM_LOGICAL_SHIFT_RIGHT_IMM() {
    u8 shift_amount = (instruction >> 7) & 0x1F;
    u8 rm = instruction & 0xF;
    return regs.r[rm] >> shift_amount;
}

u32 ARM_LOGICAL_SHIFT_RIGHT_IMMS() {
    u8 shift_amount = (instruction >> 7) & 0x1F;
    u8 rm = instruction & 0xF;
    u32 result = 0;

    if (shift_amount == 0) {
        SetConditionFlag(C_FLAG, regs.r[rm] >> 31);
    } else {
        result = regs.r[rm] >> shift_amount;
        SetConditionFlag(C_FLAG, regs.r[rm] & (1 << (shift_amount - 1)));
    }

    return result;
}

u32 ARM_LOGICAL_SHIFT_RIGHT_REGS() {
    u8 shift_amount = regs.r[(instruction >> 8) & 0xF] & 0xFF;
    u32 rm = regs.r[instruction & 0xF] + (((instruction & 0xF) == 15) ? 4 : 0);
    
    u32 result = 0;

    if (shift_amount == 0) {
        result = rm;
        // c flag stays the same
    } else if (shift_amount < 32) {
        result = rm >> shift_amount;
        SetConditionFlag(C_FLAG, rm & (1 << (shift_amount - 1)));
    } else if (shift_amount == 32) {
        result = 0;
        SetConditionFlag(C_FLAG, rm >> 31);
    } else if (shift_amount > 32) {
        result = 0;
        SetConditionFlag(C_FLAG, false);
    }

    return result;
}

u32 ARM_LOGICAL_SHIFT_RIGHT_REG() {
    u8 shift_amount = regs.r[(instruction >> 8) & 0xF] & 0xFF;
    u32 rm = regs.r[instruction & 0xF] + (((instruction & 0xF) == 15) ? 4 : 0);
    
    u32 result = 0;

    result = rm >> shift_amount;

    return result;
}

u32 ARM_ARITHMETIC_SHIFT_RIGHT_IMMS() {
    u8 shift_amount = (instruction >> 7) & 0x1F;
    u8 rm = instruction & 0xF;
    u32 result = 0;
    u8 msb = regs.r[rm] >> 31;

    if (shift_amount == 0) {
        result = 0xFFFFFFFF * msb;
        SetConditionFlag(C_FLAG, msb);
    } else {
        // shift amount > 0
        SetConditionFlag(C_FLAG, regs.r[rm] & (1 << (shift_amount - 1)));
        // perform asr
        // what happens is that rm gets shifted normally to the right but then the bits not set are set with the msb
        result = (regs.r[rm] >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
    }
    return result;
}

u32 ARM_ARITHMETIC_SHIFT_RIGHT_IMM() {
    u8 shift_amount = (instruction >> 7) & 0x1F;
    u8 rm = instruction & 0xF;
    u32 result = 0;
    u8 msb = regs.r[rm] >> 31;

    if (shift_amount == 0) {
        result = 0xFFFFFFFF * msb;
    } else {
        // shift amount > 0
        // perform asr
        // what happens is that rm gets shifted normally to the right but then the bits not set are set with the msb
        result = (regs.r[rm] >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
    }
    return result;
}

u32 ARM_ARITHMETIC_SHIFT_RIGHT_REGS() {
    u8 shift_amount = regs.r[(instruction >> 8) & 0xF] & 0xFF;
    u32 rm = regs.r[instruction & 0xF];
    if ((instruction & 0xF) == 15) {
        log_fatal("handle");
        // prefetch due to how register shifting executes after the prefetch
        rm += 4;
    }
    u32 result = 0;
    u8 msb = rm >> 31;

    if (shift_amount == 0) {
        result = rm;
    } else if (shift_amount < 32) {
        // shift amount > 0
        SetConditionFlag(C_FLAG, rm & (1 << (shift_amount - 1)));
        // perform asr
        // what happens is that rm gets shifted normally to the right but then the bits not set are set with the msb
        result = (rm >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
    } else {
        // shift amount > 32
        result = 0xFFFFFFFF * msb;
        SetConditionFlag(C_FLAG, msb);
    }
    return result;
}

u32 ARM_ARITHMETIC_SHIFT_RIGHT_REG() {
    u8 shift_amount = regs.r[(instruction >> 8) & 0xF] & 0xFF;
    u32 rm = regs.r[instruction & 0xF];
    if ((instruction & 0xF) == 15) {
        log_fatal("handle");
        // prefetch due to how register shifting executes after the prefetch
        rm += 4;
    }
    u32 result = 0;
    u8 msb = rm >> 31;

    if (shift_amount == 0) {
        result = rm;
    } else if (shift_amount < 32) {
        // shift amount > 0
        // perform asr
        // what happens is that rm gets shifted normally to the right but then the bits not set are set with the msb
        result = (rm >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
    } else {
        // shift amount > 32
        result = 0xFFFFFFFF * msb;
    }
    return result;
}

u32 ARM_ROTATE_RIGHT_IMMS() {
    u8 shift_amount = (instruction >> 7) & 0x1F;
    u8 rm = instruction & 0xF;

    u32 result = 0;
    if (shift_amount == 0) {
        // perform rotate right extend
        result = (GetConditionFlag(C_FLAG) << 31) | (regs.r[rm] >> 1);
        SetConditionFlag(C_FLAG, regs.r[rm] & 0x1);
    } else {
        // shift amount > 0
        result = (regs.r[rm] >> shift_amount) | (regs.r[rm] << (32 - shift_amount));
        SetConditionFlag(C_FLAG, regs.r[rm] & (1 << (shift_amount - 1)));
    }

    return result;
}

u32 ARM_ROTATE_RIGHT_IMM() {
    u8 shift_amount = (instruction >> 7) & 0x1F;
    u8 rm = instruction & 0xF;

    u32 result = 0;
    if (shift_amount == 0) {
        // perform rotate right extend
        result = (GetConditionFlag(C_FLAG) << 31) | (regs.r[rm] >> 1);
    } else {
        // shift amount > 0
        result = (regs.r[rm] >> shift_amount) | (regs.r[rm] << (32 - shift_amount));
    }

    return result;
}

u32 ARM_ROTATE_RIGHT_REG() {
    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u32 shift_amount = regs.r[rs] & 0x1F;
    u32 result = 0;

    if ((regs.r[rs] & 0xFF) == 0) {
        result = regs.r[rm];
    } else if (shift_amount == 0) {
        result = regs.r[rm];
    } else {
        result = (regs.r[rm] >> shift_amount) | (regs.r[rm] << (32 - shift_amount));
    }

    return result;
}

u32 ARM_SINGLE_DATA_TRANSFER_IMM() {
    return instruction & 0xFFF;
}

u32 ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM() {
    return ((instruction >> 4) & 0xF0) | (instruction & 0xF);
}

u32 ARM_HALFWORD_SIGNED_DATA_TRANSFER_REG() {
    return regs.r[instruction & 0xF];
}


// shifts for use with ldr and str
// logical shift register left by immediate shift amount
u32 ARM_RPLL() {
    u8 rm = instruction & 0xF;
    u8 shift_amount = (instruction >> 7) & 0x1F;
    
    return regs.r[rm] << shift_amount;
}

// logical shift register right by immediate shift amount
u32 ARM_RPLR() {
    u8 rm = instruction & 0xF;
    u8 shift_amount = (instruction >> 7) & 0x1F;
    if (shift_amount == 0) {
        return 0;
    } else {
        return regs.r[rm] >> shift_amount;
    }
}

// arithmetic shift register right by immediate shift amount
u32 ARM_RPAR() {
    u8 rm = instruction & 0xF;
    u8 shift_amount = (instruction >> 7) & 0x1F;
    u8 msb = regs.r[rm] >> 31;

    if (shift_amount == 0) {
        return 0xFFFFFFFF * msb;
    } else {
        return (regs.r[rm] >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
    }
}

// rotate register right by immediate shift amount
u32 ARM_RPRR() {
    u8 rm = instruction & 0xF;
    u8 shift_amount = (instruction >> 7) & 0x1F;

    if (shift_amount == 0) {
        // rotate right extended
        return (GetConditionFlag(C_FLAG) << 31) | (regs.r[rm] >> 1);
    } else {
        // rotate right
        return (regs.r[rm] >> shift_amount) | (regs.r[rm] << (32 - shift_amount));
    }
}