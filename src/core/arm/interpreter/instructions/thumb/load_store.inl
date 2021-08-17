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
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;
    u8 rm = (instruction >> 6) & 0x7;

    u8 opcode = (instruction >> 10) & 0x3;
    bool sign = instruction & (1 << 9);
    u32 address = regs.r[rn] + regs.r[rm];

    if (sign) {
        switch (opcode) {
        case 0x0:
            WriteHalf(address, regs.r[rd]);
            break;
        case 0x1:
            regs.r[rd] = (s32)(s8)ReadByte(address);
            break;
        case 0x2:
            regs.r[rd] = ReadHalf(address);
            break;
        case 0x3:
            regs.r[rd] = (s32)(s16)ReadHalf(address);
            break;
        }    
    } else {
        switch (opcode) {
        case 0x0:
            WriteWord(address, regs.r[rd]);
            break;
        case 0x1:
            WriteByte(address, regs.r[rd]);
            break;
        case 0x2: {
            regs.r[rd] = ReadWord(address);

            if (address & 0x3) {
                int shift_amount = (address & 0x3) * 8;
                regs.r[rd] = (regs.r[rd] << (32 - shift_amount)) | (regs.r[rd] >> shift_amount);
            }

            break;
        }
        case 0x3:
            regs.r[rd] = ReadByte(address);
            break;
        }
    }
    
    regs.r[15] += 2;
}

void ThumbLoadStoreImmediate() {
    u8 rd = instruction & 0x7;
    u8 rn = (instruction >> 3) & 0x7;
    u32 immediate = (instruction >> 6) & 0x1F;

    u8 opcode = (instruction >> 11) & 0x3;

    switch (opcode) {
    case 0x0:
        WriteWord(regs.r[rn] + (immediate << 2), regs.r[rd]);
        break;
    case 0x1: {
        regs.r[rd] = ReadWord(regs.r[rn] + (immediate << 2));

        if ((regs.r[rn] + (immediate << 2)) & 0x3) {
            int shift_amount = ((regs.r[rn] + (immediate << 2)) & 0x3) * 8;
            regs.r[rd] = (regs.r[rd] << (32 - shift_amount)) | (regs.r[rd] >> shift_amount);
        }

        break;
    }
    case 0x2:
        WriteByte(regs.r[rn], regs.r[rd]);
        break;
    case 0x3:
        regs.r[rd] = ReadByte(regs.r[rn]);
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
    u32 immediate = instruction & 0xFF;
    u8 rd = (instruction >> 8) & 0x7;

    bool load = instruction & (1 << 11);

    u32 address = regs.r[13] + (immediate << 2);

    if (load) {
        regs.r[rd] = ReadWord(address);

        if (address & 0x3) {
            log_fatal("handle");
            int shift_amount = (address & 0x3) * 8;
            regs.r[rd] = rotate_right(regs.r[rd], shift_amount);
        } 
    } else {
        WriteWord(address, regs.r[rd]);
    }
    
    regs.r[15] += 2;
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