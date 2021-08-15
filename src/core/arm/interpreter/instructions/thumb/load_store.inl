#pragma once

void ThumbLoadPC() {
    u32 immediate = (instruction & 0xFF) << 2;
    u8 rd = (instruction >> 8) & 0x7;
    
    // bit 1 of pc is always set to 0
    u32 address = (regs.r[15] & ~0x2) + immediate;
    regs.r[rd] = ReadWord(address);
    
    regs.r[15] += 2;
}

void ThumbLoadStore() {
    log_fatal("handle");
}

void ThumbLoadStoreImmediate() {
    log_fatal("handle");
}

void ThumbPushPop() {
    bool pclr = (instruction >> 8) & 0x1;
    bool pop = (instruction >> 11) & 0x1;

    u32 address = regs.r[13];

    if (pop) {
        for (int i = 0; i < 8; i++) {
            if (instruction & (1 << i)) {
                regs.r[i] = ReadWord(address);
                address += 4;
            }
        }

        if (pclr) {
            regs.r[15] = ReadWord(address);
            address += 4;

            if ((arch == CPUArch::ARMv4) || (regs.r[15] & 0x1)) {
                // halfword align r15 and flush pipeline
                regs.r[15] &= ~1;
                ThumbFlushPipeline();
            } else {
                // clear bit 5 of cpsr to switch to arm state
                regs.cpsr &= ~(1 << 5);
                regs.r[15] &= ~3;
                ARMFlushPipeline();
            }
        } else {
            regs.r[15] += 2;
        }

        regs.r[13] = address;
    } else {
        for (int i = 0; i < 8; i++) {
            if (instruction & (1 << i)) {
                address -= 4;
            }
        }

        if (pclr) {
            address -= 4;
        }

        regs.r[13] = address;

        for (int i = 0; i < 8; i++) {
            if (instruction & (1 << i)) {
                WriteWord(address, regs.r[i]);
                address += 4;
            }
        }

        if (pclr) {
            WriteWord(address, regs.r[14]);
        }

        regs.r[15] += 2;
    }
}