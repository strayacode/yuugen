// without writeback
INSTRUCTION(ARM_STRH_PRE, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    
    u32 address = regs.r[rn] + op2;
    if (rd == 15) {
        log_fatal("handle");
    }

    WriteHalf(address, regs.r[rd]);

    regs.r[15] += 4;
}

// without writeback
INSTRUCTION(ARM_LDRH_PRE, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    // log_warn("address is %08x with rn as %d counter %d", regs.r[rn] + op2, rn, counter);
    
    if (rn == 15) {
        log_fatal("handle");
    }
    if (rd == 15) {
        log_fatal("handle");
    }
    
    regs.r[rd] = ReadHalf(regs.r[rn] + op2);
    if (counter == 473991) {
        log_warn("address: %08x", regs.r[rn] + op2);
        log_warn("r%d is %08x", rd, regs.r[rd]);
    }
    regs.r[15] += 4;
}

// with writeback
INSTRUCTION(ARM_LDRH_PRE_WRITEBACK, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn] + op2;

    regs.r[rn] += op2;
    
    regs.r[rd] = ReadHalf(address);
    

    regs.r[15] += 4;
}

INSTRUCTION(ARM_STRH_POST, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;

    if (rd == 15) {
        log_fatal("handle");
    }

    WriteHalf(regs.r[rn], regs.r[rd]);

    // always write back to base register in post indexing
    regs.r[rn] += op2;

    regs.r[15] += 4;
}

INSTRUCTION(ARM_LDRSB_PRE, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;

    u32 address = regs.r[rn] + op2;
    // first we cast to s8 to read the data as a signed byte and then cast to s32 to sign extend to a 32 bit integer
    u32 data = (s32)(s8)ReadByte(address);

    if (rd == 15) {
        log_fatal("handle");
    }

    regs.r[rd] = data;

    regs.r[15] += 4;
}

INSTRUCTION(ARM_LDRSH_PRE, u32 op2) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;

    u32 address = regs.r[rn] + op2;
    // first we cast to s8 to read the data as a signed byte and then cast to s32 to sign extend to a 32 bit integer
    u32 data = (s32)(s16)ReadHalf(address);

    if (rd == 15) {
        log_fatal("handle");
    }

    regs.r[rd] = data;

    regs.r[15] += 4;
}