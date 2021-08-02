// without writeback
INSTRUCTION(ARM_STR_PRE, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    
    u32 address = regs.r[rn] + op2;

    WriteWord(address, regs.r[rd]);

    if (rd == 15) {
        ARMFlushPipeline();
    } else {
        regs.r[15] += 4;
    }
}

INSTRUCTION(ARM_STR_PRE_WRITEBACK, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    
    u32 address = regs.r[rn] + op2;
    if (rd == 15) {
        log_fatal("handle");
    }

    // writeback to rn
    regs.r[rn] += op2;

    WriteWord(address, regs.r[rd]);

    regs.r[15] += 4;
}

INSTRUCTION(ARM_STR_POST, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    if (rd == 15) {
        log_fatal("handle");
    }

    WriteWord(regs.r[rn], regs.r[rd]);

    // always writeback in post transfer
    regs.r[rn] += op2;

    regs.r[15] += 4;
}

// without writeback
INSTRUCTION(ARM_LDR_PRE, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn] + op2;
    u32 data = ReadWord(address);

    if (address & 0x3) {
        u8 shift_amount = (address & 0x3) * 8;
        data = (data << (32 - shift_amount)) | (data >> shift_amount);
    }

    regs.r[rd] = data;

    if (rd == 15) {
        // for armv5, bit 5 of cpsr is changed to bit 0 of data and pipeline is flushed accordingly
        // for armv4, just flush the pipeline
        if ((arch == ARMv5) && (data & 1)) {
            // switch to thumb mode
            regs.cpsr |= 1 << 5;
            regs.r[15] &= ~1;
            ThumbFlushPipeline();
        } else {
            regs.r[15] &= ~3;
            ARMFlushPipeline();
        }
    } else {
        regs.r[15] += 4;
    }
}

// with writeback
INSTRUCTION(ARM_LDR_PRE_WRITEBACK, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn] + op2;
    u32 data = ReadWord(address);

    if (address & 0x3) {
        u8 shift_amount = (address & 0x3) * 8;
        data = (data << (32 - shift_amount)) | (data >> shift_amount);
    }

    if (rd != rn) {
        regs.r[rn] += op2;
    }

    regs.r[rd] = data;

    

    if (rd == 15) {
        // for armv5, bit 5 of cpsr is changed to bit 0 of data and pipeline is flushed accordingly
        // for armv4, just flush the pipeline
        if ((arch == ARMv5) && (data & 1)) {
            // switch to thumb mode
            regs.cpsr |= 1 << 5;
            regs.r[15] &= ~1;
            ThumbFlushPipeline();
        } else {
            regs.r[15] &= ~3;
            ARMFlushPipeline();
        }
    } else {
        regs.r[15] += 4;
    }
}

INSTRUCTION(ARM_LDR_POST, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    u32 data = ReadWord(regs.r[rn]);

    if (regs.r[rn] & 0x3) {
        u8 shift_amount = (regs.r[rn] & 0x3) * 8;
        data = (data << (32 - shift_amount)) | (data >> shift_amount);
    }

    regs.r[rd] = data;  

    if (rd == 15) {
        // for armv5, bit 5 of cpsr is changed to bit 0 of data and pipeline is flushed accordingly
        // for armv4, just flush the pipeline
        if ((arch == ARMv5) && (data & 1)) {
            // switch to thumb mode
            regs.cpsr |= 1 << 5;
            regs.r[15] &= ~1;
            ThumbFlushPipeline();
        } else {
            regs.r[15] &= ~3;
            ARMFlushPipeline();
        }
    } else {
        regs.r[15] += 4;
    }
    
    if (regs.r[rn] & 0x3) {
        u8 shift_amount = (regs.r[rn] & 0x3) << 8;
        regs.r[rd] = (regs.r[rd] << (32 - shift_amount)) | (regs.r[rd] >> shift_amount);
    }

    if (rd != rn) {
        regs.r[rn] += op2;
    }
}

INSTRUCTION(ARM_STRB_POST, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    if (rd == 15) {
        log_fatal("handle");
    }

    WriteByte(regs.r[rn], regs.r[rd]);

    // always writeback in post transfer
    regs.r[rn] += op2;

    regs.r[15] += 4;
}

// without writeback
INSTRUCTION(ARM_STRB_PRE, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    if (rd == 15) {
        log_fatal("handle");
    }

    WriteByte(regs.r[rn] + op2, regs.r[rd]);

    regs.r[15] += 4;
}

INSTRUCTION(ARM_STRB_PRE_WRITEBACK, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    if (rd == 15) {
        log_fatal("handle");
    }
    regs.r[rn] += op2;
    WriteByte(regs.r[rn], regs.r[rd]);

    regs.r[15] += 4;
}

INSTRUCTION(ARM_LDRB_PRE, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;

    regs.r[rd] = ReadByte(regs.r[rn] + op2);

    if (rd == 15) {
        log_fatal("handle");
    }
    // TODO: maybe ldrb and strb use the rn == rd edgecase ill check later
    
    regs.r[15] += 4;
}

// with writeback
INSTRUCTION(ARM_LDRB_PRE_WRITEBACK, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;

    regs.r[rn] += op2;
    regs.r[rd] = ReadByte(regs.r[rn]);
    
    if (rd == 15) {
        log_fatal("handle");
    }
    
    regs.r[15] += 4;
}

INSTRUCTION(ARM_LDRB_POST, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    
    regs.r[rd] = ReadByte(regs.r[rn]);

    regs.r[rn] += op2;

    if (rd == 15) {
        log_fatal("handle");
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_LDRD_POST, u32 op2) {
    u8 rn = (instruction >> 16) & 0xF;
    regs.r[rn] += op2;

    // armv5 exclusive
    if (arch == ARMv4) {
        return;
    }
    u8 rd = (instruction >> 12) & 0xF;
    
    if (rd == 15) {
        log_fatal("handle because we cant store data in so called r16");
        
    }

    regs.r[rd] = ReadWord(regs.r[rn]);
    regs.r[rd + 1] = ReadWord(regs.r[rn] + 4);

    regs.r[15] += 4;
}