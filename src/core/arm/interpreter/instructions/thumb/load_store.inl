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
    log_fatal("handle %08x", instruction);
}

void ThumbLoadStoreImmediate() {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;
    u32 immediate = (instruction >> 6) & 0x1F;

    u32 address = regs.r[rn] + immediate;

    u8 opcode = (instruction >> 11) & 0x3;

    switch (opcode) {
    case 0x0:
        WriteWord(address, regs.r[rd]);
        break;
    case 0x1:
        regs.r[rd] = ReadWord(address);
        break;
    case 0x2:
        WriteByte(address, regs.r[rd]);
        break;
    case 0x3:
        regs.r[rd] = ReadByte(address);
        break;
    }
    
    regs.r[15] += 2;
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

void ThumbLoadStoreSPRelative() {
    log_fatal("handle");
}

void ThumbLoadStoreHalfword() {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;
    u32 immediate = (instruction >> 6) & 0x1F;
    u32 address = regs.r[rn] + (immediate << 1);

    bool load = instruction & (1 << 11);

    if (load) {
        regs.r[rd] = ReadHalf(address);
    } else {
        WriteHalf(address, regs.r[rd]);
    }

    regs.r[15] += 2;
}

void ThumbLoadStoreMultiple() {
    log_fatal("handle");
}