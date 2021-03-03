#include <stdio.h>
#include <core/arm.h>
#include <core/core.h>

ARM::ARM(Core* core, int arch) : core(core), arch(arch) {
    log_buffer = fopen("../../log-stuff/yuugen.log", "w");
}


u8 ARM::ReadByte(u32 addr) {
    if (arch == ARMv4) {
        return core->memory.ARM7ReadByte(addr);
    } else {
        return core->memory.ARM9ReadByte(addr);
    }
}

u16 ARM::ReadHalfword(u32 addr) {
    if (arch == ARMv4) {
        return core->memory.ARM7ReadHalfword(addr);
    } else {
        return core->memory.ARM9ReadHalfword(addr);
    }
}

u32 ARM::ReadWord(u32 addr) {
    if (arch == ARMv4) {
        return core->memory.ARM7ReadWord(addr);
    } else {
        return core->memory.ARM9ReadWord(addr);
    }
}

void ARM::WriteByte(u32 addr, u8 data) {
    if (arch == ARMv4) {
        core->memory.ARM7WriteByte(addr, data);
    } else {
        core->memory.ARM9WriteByte(addr, data);
    }
}

void ARM::WriteHalfword(u32 addr, u16 data) {
    if (arch == ARMv4) {
        core->memory.ARM7WriteHalfword(addr, data);
    } else {
        core->memory.ARM9WriteHalfword(addr, data);
    }
}

void ARM::WriteWord(u32 addr, u32 data) {
    if (arch == ARMv4) {
        core->memory.ARM7WriteWord(addr, data);
    } else {
        core->memory.ARM9WriteWord(addr, data);
    }
}

void ARM::Reset() {
    // initialise all registers 0 
    for (int i = 0; i < 16; i++) {
        regs.r[i] = 0;
    }

    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 7; j++) {
            regs.r_banked[i][j] = 0;
        }
        regs.spsr_banked[i] = 0;
    }

    regs.spsr = 0;

    halted = false;

    // enter supervisor mode
    regs.cpsr = 0x13;
    UpdateMode(0x13);

    pipeline[0] = pipeline[1] = 0;
}

void ARM::DirectBoot() {
    // set general purpose registers r0-r11 to 0 regardless of the cpu arch
    for (int i = 0; i < 12; i++) {
        regs.r[i] = 0;
    }

    // // TODO: double check direct boot values later
    regs.r_banked[BANK_IRQ][6] = regs.spsr_banked[BANK_IRQ] = 0;
    regs.r_banked[BANK_SVC][6] = regs.spsr_banked[BANK_SVC] = 0;

    // armv4/armv5 specific

    if (arch == CPUArchitecture::ARMv4) {
        regs.r[13] = 0x0380FD80;
        regs.r_banked[BANK_IRQ][5] = 0x0380FF80;
        regs.r_banked[BANK_SVC][5] = 0x0380FFC0;
        regs.r[12] = regs.r[14] = regs.r[15] = core->cartridge.header.arm7_entrypoint;

    } else if (arch == CPUArchitecture::ARMv5) {
        regs.r[13] = 0x03002F7C;
        regs.r_banked[BANK_IRQ][5] = 0x03003F80;
        regs.r_banked[BANK_SVC][5] = 0x03003FC0;
        regs.r[12] = regs.r[14] = regs.r[15] = core->cartridge.header.arm9_entrypoint;

    }

    // enter system mode
    regs.cpsr = 0x1F;
    UpdateMode(0x1F);

    ARMFlushPipeline();

}

bool ARM::IsARM() {
    return (!(regs.cpsr & (1 << 5)));
}

bool ARM::GetConditionFlag(int condition_flag) {
    return (regs.cpsr & (1 << condition_flag));
}

void ARM::SetConditionFlag(int condition_flag, int value) {
    if (value) {
        regs.cpsr |= (1 << condition_flag);
    } else {
        regs.cpsr &= ~(1 << condition_flag);
    }
}

bool ARM::ConditionEvaluate() {
    bool n_flag = GetConditionFlag(N_FLAG);
    bool z_flag = GetConditionFlag(Z_FLAG);
    bool c_flag = GetConditionFlag(C_FLAG);
    bool v_flag = GetConditionFlag(V_FLAG);
    
    switch (instruction >> 28) {
    case 0:
        return z_flag; // EQ
    case 1:
        return !z_flag; // NE
    case 2:
        return c_flag; // CS
    case 3:
        return !c_flag; // CC
    case 4:
        return n_flag; // MI
    case 5:
        return !n_flag; // PL
    case 6:
        return v_flag; // VS
    case 7:
        return !v_flag; // VC
    case 10:
        return (n_flag == v_flag); // GE
    case 12:
        return (!z_flag && (n_flag == v_flag)); // GT
    case 13:
        return (z_flag || (n_flag != v_flag)); // LE
    case 14:
        return true; // AL
    default:
        log_fatal("condition code %d not implemented!", instruction >> 28);
    }
}

void ARM::ARMFlushPipeline() {
    pipeline[0] = ReadWord(regs.r[15]);
    pipeline[1] = ReadWord(regs.r[15] + 4);
    regs.r[15] += 8;
}

void ARM::ThumbFlushPipeline() {
    pipeline[0] = ReadHalfword(regs.r[15]);
    pipeline[1] = ReadHalfword(regs.r[15] + 2);
    regs.r[15] += 4;
}

void ARM::ExecuteInstruction() {
    if (halted) {
        return;
    }
    // DebugRegisters();
    // TODO: maybe change arm instructions to SyntaxLikeThis instead of all caps
    // counter++;

    // if (counter == 50000) {
    //     exit(1);
    // }
    if (core->interrupt[arch].IME && ((core->interrupt[arch].IE & core->interrupt[arch].IF)) && !(regs.cpsr & (1 << 7))) {
        HandleInterrupt();
    }

    // if (arch == ARMv5) {
    //     DebugRegisters();
    // }
    // if (arch == ARMv5) {
    //     LogRegisters();
    //     // log_warn("spsr: %08x", regs.spsr);

    //     // log_debug("counter %d: cpsr cpu mode is %04x", counter, regs.cpsr & 0x1F);
    // }

    if (IsARM()) {
        if (ConditionEvaluate()) {
            u32 index = ((instruction >> 16) & 0xFF0) | ((instruction >> 4) & 0xF);
            switch (index) {
            case 0x000: case 0x008:
                return ARM_AND(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x010: case 0x018:
                return ARM_ANDS(ARM_LOGICAL_SHIFT_LEFT_IMMS());
            case 0x012: case 0x01A:
                return ARM_ANDS(ARM_LOGICAL_SHIFT_RIGHT_IMMS());
            case 0x014: case 0x01C:
                return ARM_ANDS(ARM_ARITHMETIC_SHIFT_RIGHT_IMMS());
            case 0x019:
                return ARM_MULS();
            case 0x020: case 0x028:
                return ARM_EOR(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x030: case 0x038:
                return ARM_EORS(ARM_LOGICAL_SHIFT_LEFT_IMMS());
            case 0x039:
                return ARM_MLAS();
            case 0x040: case 0x048:
                return ARM_SUB(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x04D: case 0x06D:
                return ARM_LDRD_POST(-ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM());
            case 0x050: case 0x058:
                return ARM_SUBS(ARM_LOGICAL_SHIFT_LEFT_IMMS());
            case 0x060: case 0x068:
                return ARM_RSB(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x080: case 0x088:
                return ARM_ADD(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x082: case 0x08A:
                return ARM_ADD(ARM_LOGICAL_SHIFT_RIGHT_IMM());
            case 0x090: case 0x098:
                return ARM_ADDS(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x099:
                return ARM_UMULLS();
            case 0x0A0: case 0x0A8:
                return ARM_ADC(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x0A2: case 0x0AA:
                return ARM_ADC(ARM_LOGICAL_SHIFT_RIGHT_IMM());
            case 0x0B0: case 0x0B8:
                return ARM_ADCS(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x0CB: case 0x0EB:
                return ARM_STRH_POST(ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM());
            case 0x0D0: case 0x0D8:
                return ARM_SBCS(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x0D9:
                return ARM_SMULLS();
            case 0x0F0: case 0x0F8:
                return ARM_RSCS(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x100:
                return ARM_MRS_CPSR();
            case 0x105:
                return ARM_QADD();
            case 0x108:
                return ARM_SMLABB();
            case 0x10A:
                return ARM_SMLATB();
            case 0x10C:
                return ARM_SMLABT();
            case 0x10E:
                return ARM_SMLATT();
            case 0x120:
                return ARM_MSR_CPSR_REG();
            case 0x121:
                return ARM_BX();
            case 0x123:
                return ARM_BLX_REG();
            case 0x140:
                return ARM_MRS_SPSR();
            case 0x150: case 0x158:
                return ARM_CMPS(ARM_LOGICAL_SHIFT_RIGHT_IMM());
            case 0x15B:
                return ARM_LDRH_PRE(-ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM());
            case 0x160:
                return ARM_MSR_SPSR_REG();
            case 0x161:
                return ARM_CLZ();
            case 0x170: case 0x178:
                return ARM_CMNS(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x180: case 0x188:
                return ARM_ORR(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x196: case 0x19E:
                return ARM_ORRS(ARM_ROTATE_RIGHT_IMMS());
            case 0x19B:
                return ARM_LDRH_PRE(ARM_HALFWORD_SIGNED_DATA_TRANSFER_REG());
            case 0x1A0: case 0x1A8:
                return ARM_MOV(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x1A2: case 0x1AA:
                return ARM_MOV(ARM_LOGICAL_SHIFT_RIGHT_IMM());
            case 0x1A4: case 0x1AC:
                return ARM_MOV(ARM_ARITHMETIC_SHIFT_RIGHT_IMM());
            case 0x1B0: case 0x1B8:
                return ARM_MOVS(ARM_LOGICAL_SHIFT_LEFT_IMMS());
            case 0x1B1:
                return ARM_MOVS(ARM_LOGICAL_SHIFT_LEFT_REGS());
            case 0x1B2: case 0x1BA:
                return ARM_MOVS(ARM_LOGICAL_SHIFT_RIGHT_IMMS());
            case 0x1B4: case 0x1BC:
                return ARM_MOVS(ARM_ARITHMETIC_SHIFT_RIGHT_IMMS());
            case 0x1B5:
                return ARM_MOVS(ARM_ARITHMETIC_SHIFT_RIGHT_REGS());
            case 0x1CB:
                return ARM_STRH_PRE(ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM());
            case 0x1D0: case 0x1D8:
                return ARM_BICS(ARM_LOGICAL_SHIFT_LEFT_IMMS());
            case 0x1D4: case 0x1DC:
                return ARM_BICS(ARM_ARITHMETIC_SHIFT_RIGHT_IMMS());
            case 0x1DB:
                return ARM_LDRH_PRE(ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM());
            case 0x1E0: case 0x1E8:
                return ARM_MVN(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x200: case 0x201: case 0x202: case 0x203: 
            case 0x204: case 0x205: case 0x206: case 0x207: 
            case 0x208: case 0x209: case 0x20A: case 0x20B: 
            case 0x20C: case 0x20D: case 0x20E: case 0x20F:
                return ARM_AND(ARM_DATA_PROCESSING_IMM());
            case 0x210: case 0x211: case 0x212: case 0x213: 
            case 0x214: case 0x215: case 0x216: case 0x217: 
            case 0x218: case 0x219: case 0x21A: case 0x21B: 
            case 0x21C: case 0x21D: case 0x21E: case 0x21F:
                return ARM_ANDS(ARM_DATA_PROCESSING_IMMS());
            case 0x220: case 0x221: case 0x222: case 0x223: 
            case 0x224: case 0x225: case 0x226: case 0x227: 
            case 0x228: case 0x229: case 0x22A: case 0x22B: 
            case 0x22C: case 0x22D: case 0x22E: case 0x22F:
                return ARM_EOR(ARM_DATA_PROCESSING_IMM());
            case 0x240: case 0x241: case 0x242: case 0x243: 
            case 0x244: case 0x245: case 0x246: case 0x247: 
            case 0x248: case 0x249: case 0x24A: case 0x24B: 
            case 0x24C: case 0x24D: case 0x24E: case 0x24F:
                return ARM_SUB(ARM_DATA_PROCESSING_IMM());
            case 0x250: case 0x251: case 0x252: case 0x253: 
            case 0x254: case 0x255: case 0x256: case 0x257: 
            case 0x258: case 0x259: case 0x25A: case 0x25B: 
            case 0x25C: case 0x25D: case 0x25E: case 0x25F:
                return ARM_SUBS(ARM_DATA_PROCESSING_IMM());
            case 0x280: case 0x281: case 0x282: case 0x283: 
            case 0x284: case 0x285: case 0x286: case 0x287: 
            case 0x288: case 0x289: case 0x28A: case 0x28B: 
            case 0x28C: case 0x28D: case 0x28E: case 0x28F:
                return ARM_ADD(ARM_DATA_PROCESSING_IMM());
            case 0x2D0: case 0x2D1: case 0x2D2: case 0x2D3: 
            case 0x2D4: case 0x2D5: case 0x2D6: case 0x2D7: 
            case 0x2D8: case 0x2D9: case 0x2DA: case 0x2DB: 
            case 0x2DC: case 0x2DD: case 0x2DE: case 0x2DF:
                return ARM_SBCS(ARM_DATA_PROCESSING_IMM());
            case 0x310: case 0x311: case 0x312: case 0x313: 
            case 0x314: case 0x315: case 0x316: case 0x317: 
            case 0x318: case 0x319: case 0x31A: case 0x31B: 
            case 0x31C: case 0x31D: case 0x31E: case 0x31F:
                return ARM_TSTS(ARM_DATA_PROCESSING_IMMS());
            case 0x320: case 0x321: case 0x322: case 0x323: 
            case 0x324: case 0x325: case 0x326: case 0x327: 
            case 0x328: case 0x329: case 0x32A: case 0x32B: 
            case 0x32C: case 0x32D: case 0x32E: case 0x32F:
                return ARM_MSR_CPSR_IMM();
            case 0x350: case 0x351: case 0x352: case 0x353: 
            case 0x354: case 0x355: case 0x356: case 0x357: 
            case 0x358: case 0x359: case 0x35A: case 0x35B: 
            case 0x35C: case 0x35D: case 0x35E: case 0x35F:
                return ARM_CMPS(ARM_DATA_PROCESSING_IMM());
            case 0x370: case 0x371: case 0x372: case 0x373:
            case 0x374: case 0x375: case 0x376: case 0x377:
            case 0x378: case 0x379: case 0x37A: case 0x37B:
            case 0x37C: case 0x37D: case 0x37E: case 0x37F:
                return ARM_CMNS(ARM_DATA_PROCESSING_IMM());
            case 0x380: case 0x381: case 0x382: case 0x383: 
            case 0x384: case 0x385: case 0x386: case 0x387: 
            case 0x388: case 0x389: case 0x38A: case 0x38B: 
            case 0x38C: case 0x38D: case 0x38E: case 0x38F:
                return ARM_ORR(ARM_DATA_PROCESSING_IMM());
            case 0x3A0: case 0x3A1: case 0x3A2: case 0x3A3: 
            case 0x3A4: case 0x3A5: case 0x3A6: case 0x3A7: 
            case 0x3A8: case 0x3A9: case 0x3AA: case 0x3AB: 
            case 0x3AC: case 0x3AD: case 0x3AE: case 0x3AF:
                return ARM_MOV(ARM_DATA_PROCESSING_IMM());
            case 0x3C0: case 0x3C1: case 0x3C2: case 0x3C3: 
            case 0x3C4: case 0x3C5: case 0x3C6: case 0x3C7: 
            case 0x3C8: case 0x3C9: case 0x3CA: case 0x3CB: 
            case 0x3CC: case 0x3CD: case 0x3CE: case 0x3CF:
                return ARM_BIC(ARM_DATA_PROCESSING_IMM());
            case 0x3E0: case 0x3E1: case 0x3E2: case 0x3E3: 
            case 0x3E4: case 0x3E5: case 0x3E6: case 0x3E7: 
            case 0x3E8: case 0x3E9: case 0x3EA: case 0x3EB: 
            case 0x3EC: case 0x3ED: case 0x3EE: case 0x3EF:
                return ARM_MVN(ARM_DATA_PROCESSING_IMM());
            case 0x400: case 0x401: case 0x402: case 0x403: 
            case 0x404: case 0x405: case 0x406: case 0x407: 
            case 0x408: case 0x409: case 0x40A: case 0x40B: 
            case 0x40C: case 0x40D: case 0x40E: case 0x40F:
                return ARM_STR_POST(-ARM_SINGLE_DATA_TRANSFER_IMM());
            case 0x480: case 0x481: case 0x482: case 0x483: 
            case 0x484: case 0x485: case 0x486: case 0x487: 
            case 0x488: case 0x489: case 0x48A: case 0x48B: 
            case 0x48C: case 0x48D: case 0x48E: case 0x48F:
                return ARM_STR_POST(ARM_SINGLE_DATA_TRANSFER_IMM());
            case 0x490: case 0x491: case 0x492: case 0x493: 
            case 0x494: case 0x495: case 0x496: case 0x497: 
            case 0x498: case 0x499: case 0x49A: case 0x49B: 
            case 0x49C: case 0x49D: case 0x49E: case 0x49F:
                return ARM_LDR_POST(ARM_SINGLE_DATA_TRANSFER_IMM());
            case 0x4C0: case 0x4C1: case 0x4C2: case 0x4C3: 
            case 0x4C4: case 0x4C5: case 0x4C6: case 0x4C7: 
            case 0x4C8: case 0x4C9: case 0x4CA: case 0x4CB: 
            case 0x4CC: case 0x4CD: case 0x4CE: case 0x4CF:
                return ARM_STRB_POST(ARM_SINGLE_DATA_TRANSFER_IMM());
            case 0x4D0: case 0x4D1: case 0x4D2: case 0x4D3: 
            case 0x4D4: case 0x4D5: case 0x4D6: case 0x4D7: 
            case 0x4D8: case 0x4D9: case 0x4DA: case 0x4DB: 
            case 0x4DC: case 0x4DD: case 0x4DE: case 0x4DF:
                return ARM_LDRB_POST(ARM_SINGLE_DATA_TRANSFER_IMM());
            case 0x500: case 0x501: case 0x502: case 0x503:
            case 0x504: case 0x505: case 0x506: case 0x507:
            case 0x508: case 0x509: case 0x50A: case 0x50B:
            case 0x50C: case 0x50D: case 0x50E: case 0x50F:
                return ARM_STR_PRE(-ARM_SINGLE_DATA_TRANSFER_IMM());
            case 0x510: case 0x511: case 0x512: case 0x513:
            case 0x514: case 0x515: case 0x516: case 0x517:
            case 0x518: case 0x519: case 0x51A: case 0x51B:
            case 0x51C: case 0x51D: case 0x51E: case 0x51F:
                return ARM_LDR_PRE(-ARM_SINGLE_DATA_TRANSFER_IMM());
            case 0x520: case 0x521: case 0x522: case 0x523:
            case 0x524: case 0x525: case 0x526: case 0x527:
            case 0x528: case 0x529: case 0x52A: case 0x52B:
            case 0x52C: case 0x52D: case 0x52E: case 0x52F:
                return ARM_STR_PRE_WRITEBACK(-ARM_SINGLE_DATA_TRANSFER_IMM());
            case 0x580: case 0x581: case 0x582: case 0x583:
            case 0x584: case 0x585: case 0x586: case 0x587:
            case 0x588: case 0x589: case 0x58A: case 0x58B:
            case 0x58C: case 0x58D: case 0x58E: case 0x58F:
                return ARM_STR_PRE(ARM_SINGLE_DATA_TRANSFER_IMM());
            case 0x590: case 0x591: case 0x592: case 0x593:
            case 0x594: case 0x595: case 0x596: case 0x597:
            case 0x598: case 0x599: case 0x59A: case 0x59B:
            case 0x59C: case 0x59D: case 0x59E: case 0x59F:
                return ARM_LDR_PRE(ARM_SINGLE_DATA_TRANSFER_IMM());
            case 0x5C0: case 0x5C1: case 0x5C2: case 0x5C3: 
            case 0x5C4: case 0x5C5: case 0x5C6: case 0x5C7: 
            case 0x5C8: case 0x5C9: case 0x5CA: case 0x5CB: 
            case 0x5CC: case 0x5CD: case 0x5CE: case 0x5CF:
                return ARM_STRB_PRE(ARM_SINGLE_DATA_TRANSFER_IMM());
            case 0x5D0: case 0x5D1: case 0x5D2: case 0x5D3: 
            case 0x5D4: case 0x5D5: case 0x5D6: case 0x5D7: 
            case 0x5D8: case 0x5D9: case 0x5DA: case 0x5DB: 
            case 0x5DC: case 0x5DD: case 0x5DE: case 0x5DF:
                return ARM_LDRB_PRE(ARM_SINGLE_DATA_TRANSFER_IMM());
            case 0x790: case 0x798:
                return ARM_LDR_PRE(ARM_RPLL());
            case 0x7D2: case 0x7DA:
                return ARM_LDRB_PRE(ARM_RPLR());
            case 0x890: case 0x891: case 0x892: case 0x893:
            case 0x894: case 0x895: case 0x896: case 0x897:
            case 0x898: case 0x899: case 0x89A: case 0x89B:
            case 0x89C: case 0x89D: case 0x89E: case 0x89F:
                return ARM_LDM_INCREMENT_AFTER();
            case 0x8A0: case 0x8A1: case 0x8A2: case 0x8A3:
            case 0x8A4: case 0x8A5: case 0x8A6: case 0x8A7:
            case 0x8A8: case 0x8A9: case 0x8AA: case 0x8AB:
            case 0x8AC: case 0x8AD: case 0x8AE: case 0x8AF:
                return ARM_STM_INCREMENT_AFTER_WRITEBACK();
            case 0x8B0: case 0x8B1: case 0x8B2: case 0x8B3:
            case 0x8B4: case 0x8B5: case 0x8B6: case 0x8B7:
            case 0x8B8: case 0x8B9: case 0x8BA: case 0x8BB:
            case 0x8BC: case 0x8BD: case 0x8BE: case 0x8BF:
                return ARM_LDM_INCREMENT_AFTER_WRITEBACK();
            case 0x920: case 0x921: case 0x922: case 0x923:
            case 0x924: case 0x925: case 0x926: case 0x927:
            case 0x928: case 0x929: case 0x92A: case 0x92B:
            case 0x92C: case 0x92D: case 0x92E: case 0x92F:
                return ARM_STM_DECREMENT_BEFORE_WRITEBACK();
            case 0xA00: case 0xA01: case 0xA02: case 0xA03:
            case 0xA04: case 0xA05: case 0xA06: case 0xA07:
            case 0xA08: case 0xA09: case 0xA0A: case 0xA0B:
            case 0xA0C: case 0xA0D: case 0xA0E: case 0xA0F:
            case 0xA10: case 0xA11: case 0xA12: case 0xA13:
            case 0xA14: case 0xA15: case 0xA16: case 0xA17:
            case 0xA18: case 0xA19: case 0xA1A: case 0xA1B:
            case 0xA1C: case 0xA1D: case 0xA1E: case 0xA1F:
            case 0xA20: case 0xA21: case 0xA22: case 0xA23:
            case 0xA24: case 0xA25: case 0xA26: case 0xA27:
            case 0xA28: case 0xA29: case 0xA2A: case 0xA2B:
            case 0xA2C: case 0xA2D: case 0xA2E: case 0xA2F:
            case 0xA30: case 0xA31: case 0xA32: case 0xA33:
            case 0xA34: case 0xA35: case 0xA36: case 0xA37:
            case 0xA38: case 0xA39: case 0xA3A: case 0xA3B:
            case 0xA3C: case 0xA3D: case 0xA3E: case 0xA3F:
            case 0xA40: case 0xA41: case 0xA42: case 0xA43:
            case 0xA44: case 0xA45: case 0xA46: case 0xA47:
            case 0xA48: case 0xA49: case 0xA4A: case 0xA4B:
            case 0xA4C: case 0xA4D: case 0xA4E: case 0xA4F:
            case 0xA50: case 0xA51: case 0xA52: case 0xA53:
            case 0xA54: case 0xA55: case 0xA56: case 0xA57:
            case 0xA58: case 0xA59: case 0xA5A: case 0xA5B:
            case 0xA5C: case 0xA5D: case 0xA5E: case 0xA5F:
            case 0xA60: case 0xA61: case 0xA62: case 0xA63:
            case 0xA64: case 0xA65: case 0xA66: case 0xA67:
            case 0xA68: case 0xA69: case 0xA6A: case 0xA6B:
            case 0xA6C: case 0xA6D: case 0xA6E: case 0xA6F:
            case 0xA70: case 0xA71: case 0xA72: case 0xA73:
            case 0xA74: case 0xA75: case 0xA76: case 0xA77:
            case 0xA78: case 0xA79: case 0xA7A: case 0xA7B:
            case 0xA7C: case 0xA7D: case 0xA7E: case 0xA7F:
            case 0xA80: case 0xA81: case 0xA82: case 0xA83:
            case 0xA84: case 0xA85: case 0xA86: case 0xA87:
            case 0xA88: case 0xA89: case 0xA8A: case 0xA8B:
            case 0xA8C: case 0xA8D: case 0xA8E: case 0xA8F:
            case 0xA90: case 0xA91: case 0xA92: case 0xA93:
            case 0xA94: case 0xA95: case 0xA96: case 0xA97:
            case 0xA98: case 0xA99: case 0xA9A: case 0xA9B:
            case 0xA9C: case 0xA9D: case 0xA9E: case 0xA9F:
            case 0xAA0: case 0xAA1: case 0xAA2: case 0xAA3:
            case 0xAA4: case 0xAA5: case 0xAA6: case 0xAA7:
            case 0xAA8: case 0xAA9: case 0xAAA: case 0xAAB:
            case 0xAAC: case 0xAAD: case 0xAAE: case 0xAAF:
            case 0xAB0: case 0xAB1: case 0xAB2: case 0xAB3:
            case 0xAB4: case 0xAB5: case 0xAB6: case 0xAB7:
            case 0xAB8: case 0xAB9: case 0xABA: case 0xABB:
            case 0xABC: case 0xABD: case 0xABE: case 0xABF:
            case 0xAC0: case 0xAC1: case 0xAC2: case 0xAC3:
            case 0xAC4: case 0xAC5: case 0xAC6: case 0xAC7:
            case 0xAC8: case 0xAC9: case 0xACA: case 0xACB:
            case 0xACC: case 0xACD: case 0xACE: case 0xACF:
            case 0xAD0: case 0xAD1: case 0xAD2: case 0xAD3:
            case 0xAD4: case 0xAD5: case 0xAD6: case 0xAD7:
            case 0xAD8: case 0xAD9: case 0xADA: case 0xADB:
            case 0xADC: case 0xADD: case 0xADE: case 0xADF:
            case 0xAE0: case 0xAE1: case 0xAE2: case 0xAE3:
            case 0xAE4: case 0xAE5: case 0xAE6: case 0xAE7:
            case 0xAE8: case 0xAE9: case 0xAEA: case 0xAEB:
            case 0xAEC: case 0xAED: case 0xAEE: case 0xAEF:
            case 0xAF0: case 0xAF1: case 0xAF2: case 0xAF3:
            case 0xAF4: case 0xAF5: case 0xAF6: case 0xAF7:
            case 0xAF8: case 0xAF9: case 0xAFA: case 0xAFB:
            case 0xAFC: case 0xAFD: case 0xAFE: case 0xAFF:
                // check bits 28..31
                // if it is 0b1111 then its a blx_offset
                if ((instruction & 0xF0000000) != 0xF0000000) {
                    return ARM_B();
                } else {
                    log_fatal("handle this shit");
                }
                return;
            case 0xB00: case 0xB01: case 0xB02: case 0xB03:
            case 0xB04: case 0xB05: case 0xB06: case 0xB07:
            case 0xB08: case 0xB09: case 0xB0A: case 0xB0B:
            case 0xB0C: case 0xB0D: case 0xB0E: case 0xB0F:
            case 0xB10: case 0xB11: case 0xB12: case 0xB13:
            case 0xB14: case 0xB15: case 0xB16: case 0xB17:
            case 0xB18: case 0xB19: case 0xB1A: case 0xB1B:
            case 0xB1C: case 0xB1D: case 0xB1E: case 0xB1F:
            case 0xB20: case 0xB21: case 0xB22: case 0xB23:
            case 0xB24: case 0xB25: case 0xB26: case 0xB27:
            case 0xB28: case 0xB29: case 0xB2A: case 0xB2B:
            case 0xB2C: case 0xB2D: case 0xB2E: case 0xB2F:
            case 0xB30: case 0xB31: case 0xB32: case 0xB33:
            case 0xB34: case 0xB35: case 0xB36: case 0xB37:
            case 0xB38: case 0xB39: case 0xB3A: case 0xB3B:
            case 0xB3C: case 0xB3D: case 0xB3E: case 0xB3F:
            case 0xB40: case 0xB41: case 0xB42: case 0xB43:
            case 0xB44: case 0xB45: case 0xB46: case 0xB47:
            case 0xB48: case 0xB49: case 0xB4A: case 0xB4B:
            case 0xB4C: case 0xB4D: case 0xB4E: case 0xB4F:
            case 0xB50: case 0xB51: case 0xB52: case 0xB53:
            case 0xB54: case 0xB55: case 0xB56: case 0xB57:
            case 0xB58: case 0xB59: case 0xB5A: case 0xB5B:
            case 0xB5C: case 0xB5D: case 0xB5E: case 0xB5F:
            case 0xB60: case 0xB61: case 0xB62: case 0xB63:
            case 0xB64: case 0xB65: case 0xB66: case 0xB67:
            case 0xB68: case 0xB69: case 0xB6A: case 0xB6B:
            case 0xB6C: case 0xB6D: case 0xB6E: case 0xB6F:
            case 0xB70: case 0xB71: case 0xB72: case 0xB73:
            case 0xB74: case 0xB75: case 0xB76: case 0xB77:
            case 0xB78: case 0xB79: case 0xB7A: case 0xB7B:
            case 0xB7C: case 0xB7D: case 0xB7E: case 0xB7F:
            case 0xB80: case 0xB81: case 0xB82: case 0xB83:
            case 0xB84: case 0xB85: case 0xB86: case 0xB87:
            case 0xB88: case 0xB89: case 0xB8A: case 0xB8B:
            case 0xB8C: case 0xB8D: case 0xB8E: case 0xB8F:
            case 0xB90: case 0xB91: case 0xB92: case 0xB93:
            case 0xB94: case 0xB95: case 0xB96: case 0xB97:
            case 0xB98: case 0xB99: case 0xB9A: case 0xB9B:
            case 0xB9C: case 0xB9D: case 0xB9E: case 0xB9F:
            case 0xBA0: case 0xBA1: case 0xBA2: case 0xBA3:
            case 0xBA4: case 0xBA5: case 0xBA6: case 0xBA7:
            case 0xBA8: case 0xBA9: case 0xBAA: case 0xBAB:
            case 0xBAC: case 0xBAD: case 0xBAE: case 0xBAF:
            case 0xBB0: case 0xBB1: case 0xBB2: case 0xBB3:
            case 0xBB4: case 0xBB5: case 0xBB6: case 0xBB7:
            case 0xBB8: case 0xBB9: case 0xBBA: case 0xBBB:
            case 0xBBC: case 0xBBD: case 0xBBE: case 0xBBF:
            case 0xBC0: case 0xBC1: case 0xBC2: case 0xBC3:
            case 0xBC4: case 0xBC5: case 0xBC6: case 0xBC7:
            case 0xBC8: case 0xBC9: case 0xBCA: case 0xBCB:
            case 0xBCC: case 0xBCD: case 0xBCE: case 0xBCF:
            case 0xBD0: case 0xBD1: case 0xBD2: case 0xBD3:
            case 0xBD4: case 0xBD5: case 0xBD6: case 0xBD7:
            case 0xBD8: case 0xBD9: case 0xBDA: case 0xBDB:
            case 0xBDC: case 0xBDD: case 0xBDE: case 0xBDF:
            case 0xBE0: case 0xBE1: case 0xBE2: case 0xBE3:
            case 0xBE4: case 0xBE5: case 0xBE6: case 0xBE7:
            case 0xBE8: case 0xBE9: case 0xBEA: case 0xBEB:
            case 0xBEC: case 0xBED: case 0xBEE: case 0xBEF:
            case 0xBF0: case 0xBF1: case 0xBF2: case 0xBF3:
            case 0xBF4: case 0xBF5: case 0xBF6: case 0xBF7:
            case 0xBF8: case 0xBF9: case 0xBFA: case 0xBFB:
            case 0xBFC: case 0xBFD: case 0xBFE: case 0xBFF:
                return ARM_BL();
            case 0xE01: case 0xE03: case 0xE05: case 0xE07:
            case 0xE09: case 0xE0B: case 0xE0D: case 0xE0F:
            case 0xE21: case 0xE23: case 0xE25: case 0xE27:
            case 0xE29: case 0xE2B: case 0xE2D: case 0xE2F:
            case 0xE41: case 0xE43: case 0xE45: case 0xE47:
            case 0xE49: case 0xE4B: case 0xE4D: case 0xE4F:
            case 0xE61: case 0xE63: case 0xE65: case 0xE67:
            case 0xE69: case 0xE6B: case 0xE6D: case 0xE6F:
            case 0xE81: case 0xE83: case 0xE85: case 0xE87:
            case 0xE89: case 0xE8B: case 0xE8D: case 0xE8F:
            case 0xEA1: case 0xEA3: case 0xEA5: case 0xEA7:
            case 0xEA9: case 0xEAB: case 0xEAD: case 0xEAF:
            case 0xEC1: case 0xEC3: case 0xEC5: case 0xEC7:
            case 0xEC9: case 0xECB: case 0xECD: case 0xECF:
            case 0xEE1: case 0xEE3: case 0xEE5: case 0xEE7:
            case 0xEE9: case 0xEEB: case 0xEED: case 0xEEF:
                return ARM_MCR();
            case 0xE11: case 0xE13: case 0xE15: case 0xE17:
            case 0xE19: case 0xE1B: case 0xE1D: case 0xE1F:
            case 0xE31: case 0xE33: case 0xE35: case 0xE37:
            case 0xE39: case 0xE3B: case 0xE3D: case 0xE3F:
            case 0xE51: case 0xE53: case 0xE55: case 0xE57:
            case 0xE59: case 0xE5B: case 0xE5D: case 0xE5F:
            case 0xE71: case 0xE73: case 0xE75: case 0xE77:
            case 0xE79: case 0xE7B: case 0xE7D: case 0xE7F:
            case 0xE91: case 0xE93: case 0xE95: case 0xE97:
            case 0xE99: case 0xE9B: case 0xE9D: case 0xE9F:
            case 0xEB1: case 0xEB3: case 0xEB5: case 0xEB7:
            case 0xEB9: case 0xEBB: case 0xEBD: case 0xEBF:
            case 0xED1: case 0xED3: case 0xED5: case 0xED7:
            case 0xED9: case 0xEDB: case 0xEDD: case 0xEDF:
            case 0xEF1: case 0xEF3: case 0xEF5: case 0xEF7:
            case 0xEF9: case 0xEFB: case 0xEFD: case 0xEFF:
                return ARM_MRC();
            case 0xF00: case 0xF01: case 0xF02: case 0xF03:
            case 0xF04: case 0xF05: case 0xF06: case 0xF07:
            case 0xF08: case 0xF09: case 0xF0A: case 0xF0B:
            case 0xF0C: case 0xF0D: case 0xF0E: case 0xF0F:
            case 0xF10: case 0xF11: case 0xF12: case 0xF13:
            case 0xF14: case 0xF15: case 0xF16: case 0xF17:
            case 0xF18: case 0xF19: case 0xF1A: case 0xF1B:
            case 0xF1C: case 0xF1D: case 0xF1E: case 0xF1F:
            case 0xF20: case 0xF21: case 0xF22: case 0xF23:
            case 0xF24: case 0xF25: case 0xF26: case 0xF27:
            case 0xF28: case 0xF29: case 0xF2A: case 0xF2B:
            case 0xF2C: case 0xF2D: case 0xF2E: case 0xF2F:
            case 0xF30: case 0xF31: case 0xF32: case 0xF33:
            case 0xF34: case 0xF35: case 0xF36: case 0xF37:
            case 0xF38: case 0xF39: case 0xF3A: case 0xF3B:
            case 0xF3C: case 0xF3D: case 0xF3E: case 0xF3F:
            case 0xF40: case 0xF41: case 0xF42: case 0xF43:
            case 0xF44: case 0xF45: case 0xF46: case 0xF47:
            case 0xF48: case 0xF49: case 0xF4A: case 0xF4B:
            case 0xF4C: case 0xF4D: case 0xF4E: case 0xF4F:
            case 0xF50: case 0xF51: case 0xF52: case 0xF53:
            case 0xF54: case 0xF55: case 0xF56: case 0xF57:
            case 0xF58: case 0xF59: case 0xF5A: case 0xF5B:
            case 0xF5C: case 0xF5D: case 0xF5E: case 0xF5F:
            case 0xF60: case 0xF61: case 0xF62: case 0xF63:
            case 0xF64: case 0xF65: case 0xF66: case 0xF67:
            case 0xF68: case 0xF69: case 0xF6A: case 0xF6B:
            case 0xF6C: case 0xF6D: case 0xF6E: case 0xF6F:
            case 0xF70: case 0xF71: case 0xF72: case 0xF73:
            case 0xF74: case 0xF75: case 0xF76: case 0xF77:
            case 0xF78: case 0xF79: case 0xF7A: case 0xF7B:
            case 0xF7C: case 0xF7D: case 0xF7E: case 0xF7F:
            case 0xF80: case 0xF81: case 0xF82: case 0xF83:
            case 0xF84: case 0xF85: case 0xF86: case 0xF87:
            case 0xF88: case 0xF89: case 0xF8A: case 0xF8B:
            case 0xF8C: case 0xF8D: case 0xF8E: case 0xF8F:
            case 0xF90: case 0xF91: case 0xF92: case 0xF93:
            case 0xF94: case 0xF95: case 0xF96: case 0xF97:
            case 0xF98: case 0xF99: case 0xF9A: case 0xF9B:
            case 0xF9C: case 0xF9D: case 0xF9E: case 0xF9F:
            case 0xFA0: case 0xFA1: case 0xFA2: case 0xFA3:
            case 0xFA4: case 0xFA5: case 0xFA6: case 0xFA7:
            case 0xFA8: case 0xFA9: case 0xFAA: case 0xFAB:
            case 0xFAC: case 0xFAD: case 0xFAE: case 0xFAF:
            case 0xFB0: case 0xFB1: case 0xFB2: case 0xFB3:
            case 0xFB4: case 0xFB5: case 0xFB6: case 0xFB7:
            case 0xFB8: case 0xFB9: case 0xFBA: case 0xFBB:
            case 0xFBC: case 0xFBD: case 0xFBE: case 0xFBF:
            case 0xFC0: case 0xFC1: case 0xFC2: case 0xFC3:
            case 0xFC4: case 0xFC5: case 0xFC6: case 0xFC7:
            case 0xFC8: case 0xFC9: case 0xFCA: case 0xFCB:
            case 0xFCC: case 0xFCD: case 0xFCE: case 0xFCF:
            case 0xFD0: case 0xFD1: case 0xFD2: case 0xFD3:
            case 0xFD4: case 0xFD5: case 0xFD6: case 0xFD7:
            case 0xFD8: case 0xFD9: case 0xFDA: case 0xFDB:
            case 0xFDC: case 0xFDD: case 0xFDE: case 0xFDF:
            case 0xFE0: case 0xFE1: case 0xFE2: case 0xFE3:
            case 0xFE4: case 0xFE5: case 0xFE6: case 0xFE7:
            case 0xFE8: case 0xFE9: case 0xFEA: case 0xFEB:
            case 0xFEC: case 0xFED: case 0xFEE: case 0xFEF:
            case 0xFF0: case 0xFF1: case 0xFF2: case 0xFF3:
            case 0xFF4: case 0xFF5: case 0xFF6: case 0xFF7:
            case 0xFF8: case 0xFF9: case 0xFFA: case 0xFFB:
            case 0xFFC: case 0xFFD: case 0xFFE: case 0xFFF:
                return ARM_SWI();
            default:
                log_fatal("arm%d instruction 0x%08x with index 0x%03x is unimplemented!", arch ? 9 : 7, instruction, index);
            }
        } else {
            regs.r[15] += 4;
        }
    } else {
        // using http://imrannazar.com/ARM-Opcode-Map
        u8 index = (instruction >> 8) & 0xFF;
        switch (index) {
        case 0x00: case 0x01: case 0x02: case 0x03: 
        case 0x04: case 0x05: case 0x06: case 0x07:
            return THUMB_LSL_IMM();
        case 0x08: case 0x09: case 0x0A: case 0x0B: 
        case 0x0C: case 0x0D: case 0x0E: case 0x0F:
            return THUMB_LSR_IMM();
        case 0x10: case 0x11: case 0x12: case 0x13: 
        case 0x14: case 0x15: case 0x16: case 0x17:
            return THUMB_ASR_IMM();
        case 0x18: case 0x19:
            return THUMB_ADD_REG();
        case 0x1A: case 0x1B:
            return THUMB_SUB_REG();
        case 0x1C: case 0x1D:
            return THUMB_ADD_IMM3();
        case 0x1E: case 0x1F:
            return THUMB_SUB_IMM3();
        case 0x20: case 0x21: case 0x22: case 0x23: 
        case 0x24: case 0x25: case 0x26: case 0x27:
            return THUMB_MOV_IMM();
        case 0x28: case 0x29: case 0x2A: case 0x2B: 
        case 0x2C: case 0x2D: case 0x2E: case 0x2F:
            return THUMB_CMP_IMM();
        case 0x30: case 0x31: case 0x32: case 0x33: 
        case 0x34: case 0x35: case 0x36: case 0x37:
            return THUMB_ADD_IMM();
        case 0x38: case 0x39: case 0x3A: case 0x3B: 
        case 0x3C: case 0x3D: case 0x3E: case 0x3F:
            return THUMB_SUB_IMM();
        case 0x40:
            // using the data processing opcode table with bits
            switch ((instruction >> 6) & 0x3) {
            case 0:
                return THUMB_AND_DATA_PROCESSING();
            case 1:
                log_fatal("40-1");
            case 2:
                log_fatal("40-2");
            case 3:
                log_fatal("40-3");
            default:
                log_fatal("arm%d instruction 0x%04x with index 0x%02x is unimplemented!", arch ? 9 : 7, instruction, index);
            }
            break;
        case 0x41:
            // using the data processing opcode table with bits
            switch ((instruction >> 6) & 0x3) {
            case 0:
                log_fatal("41-0");
            case 1:
                return THUMB_ADC_DATA_PROCESSING();
            case 2:
                log_fatal("41-2");
            case 3:
                log_fatal("41-3");
            default:
                log_fatal("arm%d instruction 0x%04x with index 0x%02x is unimplemented!", arch ? 9 : 7, instruction, index);
            }
            break;
        case 0x42:
            // using the data processing opcode table with bits
            switch ((instruction >> 6) & 0x3) {
            case 0:
                return THUMB_TST_DATA_PROCESSING();
            case 1:
                return THUMB_NEG_DATA_PROCESSING();
            case 2:
                return THUMB_CMP_DATA_PROCESSING();
            case 3:
                log_fatal("42-3");
            default:
                log_fatal("arm%d instruction 0x%04x with index 0x%02x is unimplemented!", arch ? 9 : 7, instruction, index);
            }
            break;
        case 0x43:
            // using the data processing opcode table with bits
            switch ((instruction >> 6) & 0x3) {
            case 0:
                return THUMB_ORR_DATA_PROCESSING();
            case 1:
                log_fatal("43-1");
            case 2:
                return THUMB_BIC_DATA_PROCESSING();
            case 3:
                log_fatal("43-3");
            default:
                log_fatal("arm%d instruction 0x%04x with index 0x%02x is unimplemented!", arch ? 9 : 7, instruction, index);
            }
            break;
        case 0x44:
            return THUMB_ADDH();
        case 0x45:
            return THUMB_CMPH();
        case 0x46:
            return THUMB_MOVH();
        case 0x47:
            return THUMB_BX_REG();
        case 0x48: case 0x49: case 0x4A: case 0x4B: 
        case 0x4C: case 0x4D: case 0x4E: case 0x4F:
            return THUMB_LDR_PC();
        case 0x50: case 0x51:
            return THUMB_STR_REG();
        case 0x52: case 0x53:
            return THUMB_STRH_REG();
        case 0x58: case 0x59:
            return THUMB_LDR_REG();
        case 0x5A: case 0x5B:
            return THUMB_LDRH_REG();
        case 0x60: case 0x61: case 0x62: case 0x63: 
        case 0x64: case 0x65: case 0x66: case 0x67:
            return THUMB_STR_IMM5();
        case 0x68: case 0x69: case 0x6A: case 0x6B: 
        case 0x6C: case 0x6D: case 0x6E: case 0x6F:
            return THUMB_LDR_IMM5();
        case 0x70: case 0x71: case 0x72: case 0x73: 
        case 0x74: case 0x75: case 0x76: case 0x77:
            return THUMB_STRB_IMM5();
        case 0x80: case 0x81: case 0x82: case 0x83: 
        case 0x84: case 0x85: case 0x86: case 0x87:
            return THUMB_STRH_IMM5();
        case 0x88: case 0x89: case 0x8A: case 0x8B: 
        case 0x8C: case 0x8D: case 0x8E: case 0x8F:
            return THUMB_LDRH_IMM5();
        case 0x90: case 0x91: case 0x92: case 0x93: 
        case 0x94: case 0x95: case 0x96: case 0x97:
            return THUMB_STR_SP();
        case 0x98: case 0x99: case 0x9A: case 0x9B: 
        case 0x9C: case 0x9D: case 0x9E: case 0x9F:
            return THUMB_LDR_SP();
        case 0xB0:
            return THUMB_ADD_SP_IMM();
        case 0xB4:
            return THUMB_PUSH();
        case 0xB5:
            return THUMB_PUSH_LR();
        case 0xBC:
            return THUMB_POP();
        case 0xBD:
            return THUMB_POP_PC();
        case 0xD0:
            return THUMB_BEQ();
        case 0xD1:
            return THUMB_BNE();
        case 0xD2:
            return THUMB_BCS();
        case 0xD5:
            return THUMB_BPL();
        case 0xD8:
            return THUMB_BHI();
        case 0xDB:
            return THUMB_BLT();
        case 0xDC:
            return THUMB_BGT();
        case 0xDD:
            return THUMB_BLE();
        case 0xE0: case 0xE1: case 0xE2: case 0xE3: 
        case 0xE4: case 0xE5: case 0xE6: case 0xE7:
            return THUMB_B(); 
        case 0xF0: case 0xF1: case 0xF2: case 0xF3: 
        case 0xF4: case 0xF5: case 0xF6: case 0xF7:
            return THUMB_BL_SETUP();
        case 0xF8: case 0xF9: case 0xFA: case 0xFB: 
        case 0xFC: case 0xFD: case 0xFE: case 0xFF:
            return THUMB_BL_OFFSET();
        default:
            log_fatal("arm%d instruction 0x%04x with index 0x%02x is unimplemented!", arch ? 9 : 7, instruction, index);
        }
    }
}

void ARM::Step() {
    // stepping the pipeline must happen before an instruction is executed incase the instruction is a branch which would flush and then step the pipeline (not correct)
    instruction = pipeline[0]; // store the current executing instruction 
    // shift the pipeline
    pipeline[0] = pipeline[1];
    // fill the 2nd item with the new instruction to be read
    if (IsARM()) {
        pipeline[1] = ReadWord(regs.r[15]);
    } else {
        pipeline[1] = ReadHalfword(regs.r[15]);
    }

    ExecuteInstruction();
}

void ARM::DebugRegisters() {
    printf("r0: %08x r1: %08x r2: %08x r3: %08x r4: %08x r5: %08x r6: %08x r7: %08x r8: %08x r9: %08x r10: %08x r11: %08x r12: %08x r13: %08x r14: %08x r15: %08x opcode: %08x\n",
        regs.r[0], regs.r[1], regs.r[2], regs.r[3], regs.r[4], regs.r[5], regs.r[6], regs.r[7], 
        regs.r[8], regs.r[9], regs.r[10], regs.r[11], regs.r[12], regs.r[13], regs.r[14], regs.r[15], instruction);
}

void ARM::LogRegisters() {
    fprintf(log_buffer, "r0: %08x r1: %08x r2: %08x r3: %08x r4: %08x r5: %08x r6: %08x r7: %08x r8: %08x r9: %08x r10: %08x r11: %08x r12: %08x r13: %08x r14: %08x r15: %08x opcode: %08x\n",
        regs.r[0], regs.r[1], regs.r[2], regs.r[3], regs.r[4], regs.r[5], regs.r[6], regs.r[7], 
        regs.r[8], regs.r[9], regs.r[10], regs.r[11], regs.r[12], regs.r[13], regs.r[14], regs.r[15], instruction);
}

int ARM::GetRegisterBank(int mode) {
    switch (mode) {
    case USR: case SYS:
        return BANK_USR;
    case FIQ:
        return BANK_FIQ;
    case IRQ:
        return BANK_IRQ;
    case SVC:
        return BANK_SVC;
    case ABT:
        return BANK_ABT;
    case UND:
        return BANK_UND;
    default:
        log_fatal("fatal unhandled mode %04x!", mode);
    }
}


void ARM::UpdateMode(int new_mode) {
    // log_debug("switch from mode %02x to %02x", regs.cpsr & 0x1F, new_mode);
    int old_bank = GetRegisterBank(regs.cpsr & 0x1F);
    int new_bank = GetRegisterBank(new_mode);
    

    // slight optimisation
    if (old_bank == new_bank) {
        return;
    }

    // only restore spsr with a banked spsr if that an spsr exists for that mode (not usr or sys)
    if ((new_mode != 0x1F) && (new_mode != 0x10)) {
        regs.spsr = regs.spsr_banked[new_bank];
        // log_debug("stored spsr with %08x from bank %08x", regs.spsr, new_bank);
    }
    
    if ((old_bank == BANK_FIQ) || (new_bank == BANK_FIQ)) {
        for (int i = 0; i < 7; i++) {
            regs.r_banked[old_bank][i] = regs.r[8 + i];
        }

        for (int i = 0; i < 7; i++) {
            regs.r[8 + i] = regs.r_banked[new_bank][i];
        }
    } else {

        regs.r_banked[old_bank][5] = regs.r[13];
        regs.r_banked[old_bank][6] = regs.r[14];
        // log_debug("stored banked r5 with %08x", regs.r[13]);
        // log_debug("stored banked r6 with %08x", regs.r[14]);

        regs.r[13] = regs.r_banked[new_bank][5];
        regs.r[14] = regs.r_banked[new_bank][6];

        // log_debug("restored r13 with %08x", regs.r[13]);
        // log_debug("restored r14 with %08x", regs.r[14]);
    }

    // finally change cpsr to reflect the new cpu mode
    regs.cpsr = (regs.cpsr & ~0x1F) | (new_mode);
}

void ARM::Halt() {
    halted = true;
}

void ARM::SendInterrupt(int interrupt) {
    // set the appropriate bit in IF
    core->interrupt[arch].IF |= (1 << interrupt);

    // check if the interrupt is enabled too
    if (core->interrupt[arch].IE & (1 << interrupt)) {
        halted = false;
    }
}

void ARM::HandleInterrupt() {
    halted = false;

    // store the cpsr in spsr_svc
    regs.spsr_banked[BANK_IRQ] = regs.cpsr;

    // enter supervisor mode
    UpdateMode(IRQ);

    // fiq interrupts state is unchanged

    // disable normal interrupts
    regs.cpsr |= (1 << 7);

    // store the address of the instruction after, depending on the cpu mode
    if (IsARM()) {
        regs.r[14] = regs.r[15] - 4;
    } else {
        // switch to arm mode
        regs.cpsr &= ~(1 << 5);
        regs.r[14] = regs.r[15];
    }

    

    
    // check the exception base and jump to the correct address in the bios
    // also only use cp15 exception base from control register if arm9
    regs.r[15] = ((arch) ? core->cp15.GetExceptionBase() : 0x00000000) + 0x18;
    
    ARMFlushPipeline();

    // slight edgecase because the new instruction doesn't get loaded yet when an interrupt is handled
    instruction = pipeline[0];
}


// coprocessor related instructions

void ARM::ARM_MRC() {
    // armv5 exclusive as it involves coprocessor transfers
    if (arch == ARMv4) {
        return;
    }

    u8 crm = instruction & 0xF;
    u8 crn = (instruction >> 16) & 0xF;
    u8 opcode2 = (instruction >> 5) & 0x7;
    u8 rd = (instruction >> 12) & 0xF;
    u32 data = core->cp15.Read(crn, crm, opcode2);

    if (rd == 15) {
        // set flags instead
        regs.cpsr = (data & 0xF0000000) | (regs.cpsr & 0x0FFFFFFF);
    } else {
        // set rd normally
        regs.r[rd] = data;
    }

    regs.r[15] += 4;
}

void ARM::ARM_MCR() {
    // armv5 exclusive as it involves coprocessor transfers
    if (arch == ARMv4) {
        return;
    }

    u8 crm = instruction & 0xF;
    u8 crn = (instruction >> 16) & 0xF;
    u8 opcode2 = (instruction >> 5) & 0x7;
    u8 rd = (instruction >> 12) & 0xF;
    core->cp15.Write(crn, crm, opcode2, regs.r[rd]);

    regs.r[15] += 4;
}

void ARM::ARM_SWI() {
    // store the cpsr in spsr_svc
    regs.spsr_banked[BANK_SVC] = regs.cpsr;

    // enter supervisor mode
    UpdateMode(SVC);

    // fiq interrupts state is unchanged

    // disable normal interrupts
    regs.cpsr |= (1 << 7);

    regs.r[14] = regs.r[15] - 4;
    // check the exception base and jump to the correct address in the bios
    // also only use cp15 exception base from control register if arm9
    regs.r[15] = ((arch) ? core->cp15.GetExceptionBase() : 0x00000000) + 0x08;
    
    ARMFlushPipeline();
}