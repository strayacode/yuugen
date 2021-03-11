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

// without writeback
INSTRUCTION(ARM_STR_PRE, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    
    u32 address = regs.r[rn] + op2;
    if (rd == 15) {
        log_fatal("handle");
    }
    WriteWord(address, regs.r[rd]);

    regs.r[15] += 4;
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
    // // TODO: make sure this is correct later
    // // i.e. not word aligned
    if (regs.r[rn] & 0x3) {
        u8 shift_amount = (regs.r[rn] & 0x3) << 8;
        regs.r[rd] = (regs.r[rd] << (32 - shift_amount)) | (regs.r[rd] >> shift_amount);
    }

    // only writeback if rn is not rd
    if (rd != rn) {
        regs.r[rn] += op2;
    }
}

// without writeback
INSTRUCTION(ARM_STRH_PRE, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    
    u32 address = regs.r[rn] + op2;
    if (rd == 15) {
        log_fatal("handle");
    }

    WriteHalfword(address, regs.r[rd]);

    regs.r[15] += 4;
}

// without writeback
INSTRUCTION(ARM_LDRH_PRE, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    if (rn == 15) {
        log_fatal("handle");
    }
    if (rd == 15) {
        log_fatal("handle");
    }

    regs.r[rd] = ReadHalfword(regs.r[rn] + op2);

    regs.r[15] += 4;
}

// with writeback
INSTRUCTION(ARM_LDRH_PRE_WRITEBACK, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn] + op2;

    regs.r[rn] += op2;
    
    regs.r[rd] = ReadHalfword(address);
    

    regs.r[15] += 4;
}

INSTRUCTION(ARM_STRH_POST, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;

    if (rd == 15) {
        log_fatal("handle");
    }

    WriteHalfword(regs.r[rn], regs.r[rd]);

    // always write back to base register in post indexing
    regs.r[rn] += op2;

    regs.r[15] += 4;
}

INSTRUCTION(ARM_MSR_CPSR_REG) {
    u8 rm = instruction & 0xF;
    bool privileged_mode = (regs.cpsr & 0x1F) != 0x10;

    if (instruction & (1 << 16)) {
        // only alter if in a privileged mode (non-user mode)
        if (privileged_mode) {
            // change bits 5..7
            regs.cpsr = (regs.cpsr & ~0xE0) | (regs.r[rm] & 0xE0);
            // update the cpu mode
            UpdateMode(regs.r[rm] & 0x1F);
        }
    }

    // only update the rest of the condition bits 8..23 if in privileged mode
    if (privileged_mode) {
        for (int i = 0; i < 2; i++) {
            if (instruction & (1 << (17 + i))) {
                regs.cpsr = (regs.cpsr & ~((0xFF00) << (i * 8))) | (regs.r[rm] & ((0xFF00) << (i * 8)));
            }
        }
    }

    // finally we can change the condition bits even if in user mode
    if (instruction & (1 << 19)) {
        regs.cpsr = (regs.cpsr & ~0xFF000000) | (regs.r[rm] & 0xFF000000);
    }
    
    regs.r[15] += 4;
}

INSTRUCTION(ARM_MSR_SPSR_REG) {
    u8 rm = instruction & 0xF;

    // spsr doesn't exist for user and system cpu mode
    bool non_user_system_mode = ((regs.spsr & 0x1F) != 0x1F) && ((regs.spsr & 0x1F) != 0x10);

    if (non_user_system_mode) {
        for (int i = 0; i < 4; i++) {
            if (instruction & (1 << (16 + i))) {
                regs.spsr = (regs.spsr & ~(0xFF << (i * 8))) | (regs.r[rm] & (0xFF << (i * 8)));
            }
        }
    }
    
    regs.r[15] += 4;
}

INSTRUCTION(ARM_MSR_CPSR_IMM) {
    u32 immediate = instruction & 0xFF;
    u8 rotate_amount = ((instruction >> 8) & 0xF) << 1;

    u32 result = (immediate >> rotate_amount) | (immediate << (32 - rotate_amount));

    bool privileged_mode = (regs.cpsr & 0x1F) != 0x10;

    if (instruction & (1 << 16)) {
        // only alter if in a privileged mode (non-user mode)
        if (privileged_mode) {
            // change bits 5..7
            regs.cpsr = (regs.cpsr & ~0xE0) | (result & 0xE0);

            // update the cpu mode
            UpdateMode(result & 0x1F);
        }
    }

    // only update the rest of the condition bits 8..23 if in privileged mode
    if (privileged_mode) {
        for (int i = 0; i < 2; i++) {
            if (instruction & (1 << (17 + i))) {
                regs.cpsr = (regs.cpsr & ~((0xFF00) << (i * 8))) | (result & ((0xFF00) << (i * 8)));
            }
        }
    }

    // finally we can change the condition bits even if in user mode
    if (instruction & (1 << 19)) {
        regs.cpsr = (regs.cpsr & ~0xFF000000) | (result & 0xFF000000);
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

    regs.r[rd] = regs.spsr;

    regs.r[15] += 4;
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

INSTRUCTION(ARM_LDRB_PRE, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;

    // printf("reading from address %08x\n", regs.r[rn] + op2);

    regs.r[rd] = ReadByte(regs.r[rn] + op2);

    if (rd == 15) {
        log_fatal("handle");
    }
    // TODO: maybe ldrb and strb use the rn == rd edgecase ill check later
    
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



INSTRUCTION(ARM_STM_DECREMENT_BEFORE_WRITEBACK) {
    u8 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn];

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            address -= 4;
        }
    }

    u32 writeback = address;

    // subtract offset from base
    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) { 
            // write register to address
            WriteWord(address, regs.r[i]);
            // pre decrement the address
            address += 4;
        }
    }

    // writeback to base register
    regs.r[rn] = writeback;

    regs.r[15] += 4;
}

INSTRUCTION(ARM_LDM_INCREMENT_BEFORE_WRITEBACK) {
    u8 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn];

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            address += 4;
            regs.r[i] = ReadWord(address);
        }
    }


    // if rn is in rlist:
    // if arm9 writeback if rn is the only register or not the last register in rlist
    // if arm7 then no writeback if rn in rlist

    if (!(instruction & (1 << rn)) || ((arch == ARMv5) && ((instruction & 0xFFFF) == (1 << rn)) || !((instruction & 0xFFFF) >> rn))) {
        regs.r[rn] = address;
    }
    
    if (instruction & (1 << 15)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_LDM_INCREMENT_AFTER_WRITEBACK) {
    u8 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn];
    
    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            regs.r[i] = ReadWord(address);
            address += 4;
        }
    }


    // if rn is in rlist:
    // if arm9 writeback if rn is the only register or not the last register in rlist
    // if arm7 then no writeback if rn in rlist

    if ((!(instruction & (1 << rn))) || ((arch == ARMv5) && ((instruction & 0xFFFF) == (1 << rn)) || !((instruction & 0xFFFF) >> rn))) {
        regs.r[rn] = address;
    }
    
    if (instruction & (1 << 15)) {
        // log_fatal("handle");
        // handle arm9 behaviour
        if (arch == ARMv5) {
            if (regs.r[15] & 0x1) {
                // switch to thumb mode
                regs.cpsr |= (1 << 5);

                // halfword align the address
                regs.r[15] &= ~1;

                ThumbFlushPipeline();
            } else {
                // word align the address
                regs.r[15] &= ~3;

                ARMFlushPipeline();
            }
        } else {
            // word align the address
            regs.r[15] &= ~3;

            ARMFlushPipeline();
        }
    } else {
        regs.r[15] += 4;
    }  
}

INSTRUCTION(ARM_LDM_INCREMENT_AFTER) {
    u8 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn];
    
    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            regs.r[i] = ReadWord(address);
            address += 4;
        }
    }
    
    if (instruction & (1 << 15)) {
        log_fatal("handle");
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_STM_INCREMENT_AFTER_WRITEBACK) {
    u8 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn];

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            WriteWord(address, regs.r[i]);
            address += 4;
        }
    }

    regs.r[rn] = address;

    regs.r[15] += 4;
}

INSTRUCTION(ARM_LDRD_POST, u32 op2) {
    // armv5 exclusive
    if (arch == ARMv4) {
        return;
    }

    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;

    if (rd == 15) {
        log_fatal("handle because we cant store data in so called r16");
        
    }

    regs.r[rd] = ReadWord(regs.r[rn]);
    regs.r[rd + 1] = ReadWord(regs.r[rn] + 4);

    regs.r[rn] += op2;

    regs.r[15] += 4;
}