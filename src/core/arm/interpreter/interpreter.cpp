#include <core/arm/interpreter/interpreter.h>
#include <core/hw/cp15/cp15.h>

Interpreter::Interpreter(MemoryBase& memory, CPUArch arch, CP15* cp15) : CPUBase(memory, arch), cp15(cp15) {
    GenerateConditionTable();
    log_file = std::make_unique<LogFile>("../../log-stuff/new-yuugen.log");
}

Interpreter::~Interpreter() {

}

void Interpreter::Reset() {
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
    regs.cpsr = 0xD3;
    SwitchMode(SVC);

    pipeline[0] = pipeline[1] = 0;
}

void Interpreter::Run(int cycles) {
    while (cycles--) {
        // stepping the pipeline must happen before an instruction is executed incase the instruction is a branch which would flush and then step the pipeline (not correct)
        instruction = pipeline[0]; // store the current executing instruction 
        
        // shift the pipeline
        pipeline[0] = pipeline[1];

        // TODO: align r15
        if (IsARM()) {
            pipeline[1] = ReadWord(regs.r[15]);
        } else {
            pipeline[1] = ReadHalf(regs.r[15]);
        }

        Execute();
    }
}

void Interpreter::DirectBoot(u32 entrypoint) {
    // set general purpose registers r0-r11 to 0 regardless of the cpu arch
    for (int i = 0; i < 12; i++) {
        regs.r[i] = 0;
    }

    regs.r_banked[BANK_IRQ][6] = 0;
    regs.spsr_banked[BANK_IRQ] = 0;

    regs.r_banked[BANK_SVC][6] = 0;
    regs.spsr_banked[BANK_SVC] = 0;

    regs.r[12] = regs.r[14] = regs.r[15] = entrypoint;

    // TODO: move out of interpreter class
    // armv4/armv5 specific
    if (arch == CPUArch::ARMv4) {
        regs.r[13] = 0x0380FD80;
        regs.r_banked[BANK_IRQ][5] = 0x0380FF80;
        regs.r_banked[BANK_SVC][5] = 0x0380FFC0;
    } else if (arch == CPUArch::ARMv5) {
        regs.r[13] = 0x03002F7C;
        regs.r_banked[BANK_IRQ][5] = 0x03003F80;
        regs.r_banked[BANK_SVC][5] = 0x03003FC0;
    }

    // enter system mode
    regs.cpsr = 0xDF;
    SwitchMode(SYS);

    ARMFlushPipeline();
}

void Interpreter::FirmwareBoot() {
    if (arch == CPUArch::ARMv5) {
        // make arm9 start at arm9 bios
        regs.r[15] = 0xFFFF0000;
    } else {
        // make arm7 start at arm7 bios
        regs.r[15] = 0x00000000;
    }
    
    regs.cpsr = 0xD3;
    SwitchMode(SVC);

    ARMFlushPipeline();
}

void Interpreter::ARMFlushPipeline() {
    regs.r[15] &= ~3;
    pipeline[0] = ReadWord(regs.r[15]);
    pipeline[1] = ReadWord(regs.r[15] + 4);
    regs.r[15] += 8;
}

void Interpreter::ThumbFlushPipeline() {
    regs.r[15] &= ~1;
    pipeline[0] = ReadHalf(regs.r[15]);
    pipeline[1] = ReadHalf(regs.r[15] + 2);
    regs.r[15] += 4;
}

auto Interpreter::GetCurrentSPSR() -> u32 {
    switch (regs.cpsr & 0x1F) {
    case FIQ:
        return regs.spsr_banked[BANK_FIQ];
    case IRQ:
        return regs.spsr_banked[BANK_IRQ];
    case SVC:
        return regs.spsr_banked[BANK_SVC];
    case ABT:
        return regs.spsr_banked[BANK_ABT];
    case UND:
        return regs.spsr_banked[BANK_UND];
    default:
        log_warn("[Interpreter] Mode %02x doesn't have an spsr", regs.cpsr & 0x1F);
        return 0;
    }
}

void Interpreter::SetCurrentSPSR(u32 data) {
    switch (regs.cpsr & 0x1F) {
    case FIQ:
        regs.spsr_banked[BANK_FIQ] = data;
        break;
    case IRQ:
        regs.spsr_banked[BANK_IRQ] = data;
        break;
    case SVC:
        regs.spsr_banked[BANK_SVC] = data;
        break;
    case ABT:
        regs.spsr_banked[BANK_ABT] = data;
        break;
    case UND:
        regs.spsr_banked[BANK_UND] = data;
        break;
    default:
        log_warn("[Interpreter] Mode %02x doesn't have an spsr", regs.cpsr & 0x1F);
        return;
    }
}

bool Interpreter::HasSPSR() {
    u8 current_mode = regs.cpsr & 0x1F;

    return (current_mode != USR && current_mode != SYS);
}

bool Interpreter::PrivilegedMode() {
    u8 current_mode = regs.cpsr & 0x1F;
    return (current_mode != USR);
}

bool Interpreter::IsARM() {
    return (!(regs.cpsr & (1 << 5)));
}

// void Interpreter::UnimplementedInstruction() {
//     log_fatal("[Interpreter] Instruction %08x is unimplemented at r15 = %08x", instruction, regs.r[15]);
// }

void Interpreter::GenerateConditionTable() {
    // iterate through each combination of bits 28..31
    for (int i = 0; i < 16; i++) {
        bool n_flag = i & 8;
        bool z_flag = i & 4;
        bool c_flag = i & 2;
        bool v_flag = i & 1;

        condition_table[CONDITION_EQ][i] = z_flag;
        condition_table[CONDITION_NE][i] = !z_flag;
        condition_table[CONDITION_CS][i] = c_flag;
        condition_table[CONDITION_CC][i] = !c_flag;
        condition_table[CONDITION_MI][i] = n_flag;
        condition_table[CONDITION_PL][i] = !n_flag;
        condition_table[CONDITION_VS][i] = v_flag;
        condition_table[CONDITION_VC][i] = !v_flag;
        condition_table[CONDITION_HI][i] = (c_flag && !z_flag);
        condition_table[CONDITION_LS][i] = (!c_flag || z_flag);
        condition_table[CONDITION_GE][i] = (n_flag == v_flag);
        condition_table[CONDITION_LT][i] = (n_flag != v_flag);
        condition_table[CONDITION_GT][i] = (!z_flag && (n_flag == v_flag));
        condition_table[CONDITION_LE][i] = (z_flag || (n_flag != v_flag));
        condition_table[CONDITION_AL][i] = true;
        // this one depends on cpu arch and instruction
        condition_table[CONDITION_NV][i] = true;
    }
}

bool Interpreter::ConditionEvaluate(u8 condition) {
    if (condition == CONDITION_NV) {
        if (arch == CPUArch::ARMv5) {
            // using arm decoding table this can only occur for branch and branch with link and change to thumb
            // so where bits 25..27 is 0b101
            if ((instruction & 0x0E000000) == 0xA000000) {
                return true;
            }
        } else {
            return false;
        }
    }

    return condition_table[condition][regs.cpsr >> 28];
}

void Interpreter::SwitchMode(u8 new_mode) {
    if ((regs.cpsr & 0x1F) == new_mode) {
        return;
    }

    // store the current regs in banked regs for old mode,
    // so when we switch back to old mode we can "restore" those regs
    // only store spsr in spsr bank if not in privileged mode
    switch (regs.cpsr & 0x1F) {
    case USR: case SYS:
        break;
    case FIQ:
        for (int i = 0; i < 7; i++) {
            std::swap(regs.r_banked[BANK_FIQ][i], regs.r[i + 8]);
        }

        break;
    case IRQ:
        std::swap(regs.r_banked[BANK_IRQ][5], regs.r[13]);
        std::swap(regs.r_banked[BANK_IRQ][6], regs.r[14]);
        break;
    case SVC:
        std::swap(regs.r_banked[BANK_SVC][5], regs.r[13]);
        std::swap(regs.r_banked[BANK_SVC][6], regs.r[14]);
        break;
    case ABT:
        std::swap(regs.r_banked[BANK_ABT][5], regs.r[13]);
        std::swap(regs.r_banked[BANK_ABT][6], regs.r[14]);
        break;
    case UND:
        std::swap(regs.r_banked[BANK_UND][5], regs.r[13]);
        std::swap(regs.r_banked[BANK_UND][6], regs.r[14]);
        break;
    }

    // take values from the banked regs and put in general regs
    // only store in spsr from spsr banked if not in privileged mode
    switch (new_mode) {
    case USR: case SYS:
        break;
    case FIQ:
        for (int i = 0; i < 7; i++) {
            std::swap(regs.r[i + 8], regs.r_banked[BANK_FIQ][i]);
        }
        
        break;
    case IRQ:
        std::swap(regs.r[13], regs.r_banked[BANK_IRQ][5]);
        std::swap(regs.r[14], regs.r_banked[BANK_IRQ][6]);
        break;
    case SVC:
        std::swap(regs.r[13], regs.r_banked[BANK_SVC][5]);
        std::swap(regs.r[14], regs.r_banked[BANK_SVC][6]);
        break;
    case ABT:
        std::swap(regs.r[13], regs.r_banked[BANK_ABT][5]);
        std::swap(regs.r[14], regs.r_banked[BANK_ABT][6]);
        break;
    case UND:
        std::swap(regs.r[13], regs.r_banked[BANK_UND][5]);
        std::swap(regs.r[14], regs.r_banked[BANK_UND][6]);
        break;
    }

    // finally update cpsr to have the new mode
    regs.cpsr = (regs.cpsr & ~0x1F) | new_mode;
}

bool Interpreter::GetConditionFlag(int condition_flag) {
    return (regs.cpsr & (1 << condition_flag));
}

void Interpreter::SetConditionFlag(int condition_flag, int value) {
    if (value) {
        regs.cpsr |= (1 << condition_flag);
    } else {
        regs.cpsr &= ~(1 << condition_flag);
    }
}

void Interpreter::LogRegisters() {
    log_file->Log("r0: %08x r1: %08x r2: %08x r3: %08x r4: %08x r5: %08x r6: %08x r7: %08x r8: %08x r9: %08x r10: %08x r11: %08x r12: %08x r13: %08x r14: %08x r15: %08x opcode: %08x cpsr: %08x\n",
        regs.r[0], regs.r[1], regs.r[2], regs.r[3], regs.r[4], regs.r[5], regs.r[6], regs.r[7], 
        regs.r[8], regs.r[9], regs.r[10], regs.r[11], regs.r[12], regs.r[13], regs.r[14], regs.r[15], instruction, regs.cpsr);
}

auto Interpreter::ReadByte(u32 addr) -> u8 {
    return memory.FastRead<u8>(addr);
}

auto Interpreter::ReadHalf(u32 addr) -> u16 {
    return memory.FastRead<u16>(addr);
}

auto Interpreter::ReadWord(u32 addr) -> u32 {
    return memory.FastRead<u32>(addr);
}

void Interpreter::WriteByte(u32 addr, u8 data) {
    memory.FastWrite<u8>(addr, data);
}

void Interpreter::WriteHalf(u32 addr, u16 data) {
    memory.FastWrite<u16>(addr, data);
}

void Interpreter::WriteWord(u32 addr, u32 data) {
    memory.FastWrite<u32>(addr, data);
}

void Interpreter::HandleInterrupt() {
    halted = false;
    
    regs.spsr_banked[BANK_IRQ] = regs.cpsr;

    SwitchMode(IRQ);

    regs.cpsr |= (1 << 7);

    if (IsARM()) {
        regs.r[14] = regs.r[15] - 4;
    } else {
        regs.cpsr &= ~(1 << 5);
        regs.r[14] = regs.r[15];
    }

    // check the exception base and jump to the correct address in the bios
    // also only use cp15 exception base from control register if arm9
    regs.r[15] = ((arch == CPUArch::ARMv5) ? cp15->GetExceptionBase() : 0x00000000) + 0x18;
    
    ARMFlushPipeline();

    // slight edgecase because the new instruction doesn't get loaded yet when an interrupt is handled
    instruction = pipeline[0];
}

void Interpreter::Execute() {
    if (halted) {
        return;
    }

    if (ime && (ie & irf) && !(regs.cpsr & (1 << 7))) {
        HandleInterrupt();
    }

    if (IsARM()) {
        if (ConditionEvaluate(instruction >> 28)) {
            u32 index = ((instruction >> 16) & 0xFF0) | ((instruction >> 4) & 0xF);
            switch (index) {
            case 0x000: case 0x008:
                return ARM_AND(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x001:
                return ARM_AND(ARM_LOGICAL_SHIFT_LEFT_REG());
            case 0x002: case 0x00A:
                return ARM_AND(ARM_LOGICAL_SHIFT_RIGHT_IMM());
            case 0x003:
                return ARM_AND(ARM_LOGICAL_SHIFT_RIGHT_REG());
            case 0x004: case 0x00C:
                return ARM_AND(ARM_ARITHMETIC_SHIFT_RIGHT_IMM());
            case 0x005:
                return ARM_AND(ARM_ARITHMETIC_SHIFT_RIGHT_REG());
            case 0x006: case 0x00E:
                return ARM_AND(ARM_ROTATE_RIGHT_IMM());
            case 0x009:
                return ARM_MUL();
            case 0x010: case 0x018:
                return ARM_ANDS(ARM_LOGICAL_SHIFT_LEFT_IMMS());
            case 0x011:
                return ARM_ANDS(ARM_LOGICAL_SHIFT_LEFT_REGS());
            case 0x012: case 0x01A:
                return ARM_ANDS(ARM_LOGICAL_SHIFT_RIGHT_IMMS());
            case 0x014: case 0x01C:
                return ARM_ANDS(ARM_ARITHMETIC_SHIFT_RIGHT_IMMS());
            case 0x019:
                return ARM_MULS();
            case 0x020: case 0x028:
                return ARM_EOR(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x021:
                return ARM_EOR(ARM_LOGICAL_SHIFT_LEFT_REG());
            case 0x022: case 0x02A:
                return ARM_EOR(ARM_LOGICAL_SHIFT_RIGHT_IMM());
            case 0x023:
                return ARM_EOR(ARM_LOGICAL_SHIFT_RIGHT_REG());
            case 0x024: case 0x02C:
                return ARM_EOR(ARM_ARITHMETIC_SHIFT_RIGHT_IMM());
            case 0x026: case 0x2E:
                return ARM_EOR(ARM_ROTATE_RIGHT_IMM());
            case 0x027:
                return ARM_EOR(ARM_ROTATE_RIGHT_REG());
            case 0x029:
                return ARM_MLA();
            case 0x030: case 0x038:
                return ARM_EORS(ARM_LOGICAL_SHIFT_LEFT_IMMS());
            case 0x034: case 0x03C:
                return ARM_EORS(ARM_ARITHMETIC_SHIFT_RIGHT_IMMS());
            case 0x039:
                return ARM_MLAS();
            case 0x040: case 0x048:
                return ARM_SUB(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x042: case 0x04A:
                return ARM_SUB(ARM_LOGICAL_SHIFT_RIGHT_IMM());
            case 0x044: case 0x04C:
                return ARM_SUB(ARM_ARITHMETIC_SHIFT_RIGHT_IMM());
            case 0x04D: case 0x06D:
                return ARM_LDRD_POST(-ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM());
            case 0x050: case 0x058:
                return ARM_SUBS(ARM_LOGICAL_SHIFT_LEFT_IMMS());
            case 0x052: case 0x05A:
                return ARM_SUBS(ARM_LOGICAL_SHIFT_RIGHT_IMMS());
            case 0x054: case 0x05C:
                return ARM_SUBS(ARM_ARITHMETIC_SHIFT_RIGHT_IMMS());
            case 0x05B: case 0x07B:
                return ARM_LDRH_POST(-ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM());
            case 0x05F: case 0x07F:
                return ARM_LDRSH_POST(-ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM());
            case 0x060: case 0x068:
                return ARM_RSB(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x062: case 0x06A:
                return ARM_RSB(ARM_LOGICAL_SHIFT_RIGHT_IMM());
            case 0x064: case 0x06C:
                return ARM_RSB(ARM_ARITHMETIC_SHIFT_RIGHT_IMM());
            case 0x070: case 0x078:
                return ARM_RSBS(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x072: case 0x07A:
                return ARM_RSBS(ARM_LOGICAL_SHIFT_RIGHT_IMM());
            case 0x080: case 0x088:
                return ARM_ADD(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x081:
                return ARM_ADD(ARM_LOGICAL_SHIFT_LEFT_REG());
            case 0x082: case 0x08A:
                return ARM_ADD(ARM_LOGICAL_SHIFT_RIGHT_IMM());
            case 0x083:
                return ARM_ADD(ARM_LOGICAL_SHIFT_RIGHT_REG());
            case 0x084: case 0x08C:
                return ARM_ADD(ARM_ARITHMETIC_SHIFT_RIGHT_IMM());
            case 0x085:
                return ARM_ADD(ARM_ARITHMETIC_SHIFT_RIGHT_REG());
            case 0x086: case 0x08E:
                return ARM_ADD(ARM_ROTATE_RIGHT_IMM());
            case 0x089:
                return ARM_UMULL();
            case 0x08B: case 0x0AB:
                return ARM_STRH_POST(ARM_HALFWORD_SIGNED_DATA_TRANSFER_REG());
            case 0x090: case 0x098:
                return ARM_ADDS(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x092: case 0x09A:
                return ARM_ADDS(ARM_LOGICAL_SHIFT_RIGHT_IMM());
            case 0x094: case 0x09C:
                return ARM_ADDS(ARM_ARITHMETIC_SHIFT_RIGHT_IMM());
            case 0x095:
                return ARM_ADDS(ARM_ARITHMETIC_SHIFT_RIGHT_REG());
            case 0x096: case 0x09E:
                return ARM_ADDS(ARM_ROTATE_RIGHT_IMM());
            case 0x099:
                return ARM_UMULLS();
            case 0x0A0: case 0x0A8:
                return ARM_ADC(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x0A9:
                return ARM_UMLAL();
            case 0x0A2: case 0x0AA:
                return ARM_ADC(ARM_LOGICAL_SHIFT_RIGHT_IMM());
            case 0x0A4: case 0x0AC:
                return ARM_ADC(ARM_ARITHMETIC_SHIFT_RIGHT_IMM());
            case 0x0B0: case 0x0B8:
                return ARM_ADCS(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x0B9:
                return ARM_UMLALS();
            case 0x0C0: case 0x0C8:
                return ARM_SBC(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x0C4: case 0x0CC:
                return ARM_SBC(ARM_ARITHMETIC_SHIFT_RIGHT_IMM());
            case 0x0C9:
                return ARM_SMULL();
            case 0x0CB: case 0x0EB:
                return ARM_STRH_POST(ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM());
            case 0x0D0: case 0x0D8:
                return ARM_SBCS(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x0D9:
                return ARM_SMULLS();
            case 0x0DB: case 0x0FB:
                return ARM_LDRH_POST(ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM());
            case 0x0DD: case 0x0FD:
                return ARM_LDRSB_POST(ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM());
            case 0x0DF: case 0x0FF:
                return ARM_LDRSH_POST(ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM());
            case 0x0E4: case 0x0EC:
                return ARM_RSC(ARM_ARITHMETIC_SHIFT_RIGHT_IMM());
            case 0x0E9:
                return ARM_SMLAL();
            case 0x0F0: case 0x0F8:
                return ARM_RSCS(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x0F9:
                return ARM_SMLALS();
            case 0x100:
                return ARM_MRS_CPSR();
            case 0x105:
                return ARM_QADD();
            case 0x108:
                return ARM_SMLABB();
            case 0x109:
                return ARM_SWP();
            case 0x10A:
                return ARM_SMLATB();
            case 0x10C:
                return ARM_SMLABT();
            case 0x10E:
                return ARM_SMLATT();
            case 0x110: case 0x118:
                return ARM_TSTS(ARM_LOGICAL_SHIFT_LEFT_IMMS());
            case 0x111:
                return ARM_TSTS(ARM_LOGICAL_SHIFT_LEFT_REGS());
            case 0x112: case 0x11A:
                return ARM_TSTS(ARM_LOGICAL_SHIFT_RIGHT_IMMS());
            case 0x113:
                return ARM_TSTS(ARM_LOGICAL_SHIFT_RIGHT_REGS());
            case 0x114: case 0x11C:
                return ARM_TSTS(ARM_ARITHMETIC_SHIFT_RIGHT_IMMS());
            case 0x115:
                return ARM_TSTS(ARM_ARITHMETIC_SHIFT_RIGHT_REGS());
            case 0x11B:
                return ARM_LDRH_PRE(-ARM_HALFWORD_SIGNED_DATA_TRANSFER_REG());
            case 0x11D:
                return ARM_LDRSB_PRE(-ARM_HALFWORD_SIGNED_DATA_TRANSFER_REG());
            case 0x11F:
                return ARM_LDRSH_PRE(-ARM_HALFWORD_SIGNED_DATA_TRANSFER_REG());
            case 0x120:
                return ARM_MSR_CPSR_REG();
            case 0x121:
                return ARM_BX();
            case 0x123:
                return ARM_BLX_REG();
            case 0x125:
                return ARM_QSUB();
            case 0x128:
                return ARM_SMLAWB();
            case 0x12A:
                return ARM_SMULWB();
            case 0x12C:
                return ARM_SMLAWT();
            case 0x12E:
                return ARM_SMULWT();
            case 0x130: case 0x138:
                return ARM_TEQS(ARM_LOGICAL_SHIFT_LEFT_IMMS());
            case 0x140:
                return ARM_MRS_SPSR();
            case 0x145:
                return ARM_QDADD();
            case 0x148:
                return ARM_SMLALBB();
            case 0x149:
                return ARM_SWPB();
            case 0x14A:
                return ARM_SMLALTB();
            case 0x14C:
                return ARM_SMLALBT();
            case 0x14E:
                return ARM_SMLALTT();
            case 0x14B:
                return ARM_STRH_PRE(-ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM());
            case 0x150: case 0x158:
                return ARM_CMPS(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x152: case 0x15A:
                return ARM_CMPS(ARM_LOGICAL_SHIFT_RIGHT_IMM());
            case 0x154: case 0x15C:
                return ARM_CMPS(ARM_ARITHMETIC_SHIFT_RIGHT_IMM());
            case 0x155:
                return ARM_CMPS(ARM_ARITHMETIC_SHIFT_RIGHT_REG());
            case 0x156: case 0x15E:
                return ARM_CMPS(ARM_ROTATE_RIGHT_IMM());
            case 0x15B:
                return ARM_LDRH_PRE(-ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM());
            case 0x15D:
                return ARM_LDRSB_PRE(-ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM());
            case 0x15F:
                return ARM_LDRSH_PRE(-ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM());
            case 0x160:
                return ARM_MSR_SPSR_REG();
            case 0x161:
                return ARM_CLZ();
            case 0x165:
                return ARM_QDSUB();
            case 0x168:
                return ARM_SMULBB();
            case 0x16A:
                return ARM_SMULTB();
            case 0x16B:
                return ARM_STRH_PRE_WRITEBACK(-ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM());
            case 0x16C:
                return ARM_SMULBT();
            case 0x16E:
                return ARM_SMULTT();
            case 0x170: case 0x178:
                return ARM_CMNS(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x17B:
                return ARM_LDRH_PRE_WRITEBACK(-ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM());
            case 0x17D:
                return ARM_LDRSB_PRE_WRITEBACK(-ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM());
            case 0x180: case 0x188:
                return ARM_ORR(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x181:
                return ARM_ORR(ARM_LOGICAL_SHIFT_LEFT_REG());
            case 0x182: case 0x18A:
                return ARM_ORR(ARM_LOGICAL_SHIFT_RIGHT_IMM());
            case 0x183:
                return ARM_ORR(ARM_LOGICAL_SHIFT_RIGHT_REG());
            case 0x184: case 0x18C:
                return ARM_ORR(ARM_ARITHMETIC_SHIFT_RIGHT_IMM());
            case 0x185:
                return ARM_ORR(ARM_ARITHMETIC_SHIFT_RIGHT_REG());
            case 0x186: case 0x18E:
                return ARM_ORR(ARM_ROTATE_RIGHT_IMM());
            case 0x18B:
                return ARM_STRH_PRE(ARM_HALFWORD_SIGNED_DATA_TRANSFER_REG());
            case 0x190: case 0x198:
                return ARM_ORRS(ARM_LOGICAL_SHIFT_LEFT_IMMS());
            case 0x192: case 0x19A:
                return ARM_ORRS(ARM_LOGICAL_SHIFT_RIGHT_IMMS());
            case 0x196: case 0x19E:
                return ARM_ORRS(ARM_ROTATE_RIGHT_IMMS());
            case 0x19B:
                return ARM_LDRH_PRE(ARM_HALFWORD_SIGNED_DATA_TRANSFER_REG());
            case 0x19D:
                return ARM_LDRSB_PRE(ARM_HALFWORD_SIGNED_DATA_TRANSFER_REG());
            case 0x19F:
                return ARM_LDRSH_PRE(ARM_HALFWORD_SIGNED_DATA_TRANSFER_REG());
            case 0x1A0: case 0x1A8:
                return ARM_MOV(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x1A1:
                return ARM_MOV(ARM_LOGICAL_SHIFT_LEFT_REG());
            case 0x1A2: case 0x1AA:
                return ARM_MOV(ARM_LOGICAL_SHIFT_RIGHT_IMM());
            case 0x1A3:
                return ARM_MOV(ARM_LOGICAL_SHIFT_RIGHT_REG());
            case 0x1A4: case 0x1AC:
                return ARM_MOV(ARM_ARITHMETIC_SHIFT_RIGHT_IMM());
            case 0x1A5:
                return ARM_MOV(ARM_ARITHMETIC_SHIFT_RIGHT_REG());
            case 0x1A6: case 0x1AE:
                return ARM_MOV(ARM_ROTATE_RIGHT_IMM());
            case 0x1A7:
                return ARM_MOV(ARM_ROTATE_RIGHT_REG());
            case 0x1B0: case 0x1B8:
                return ARM_MOVS(ARM_LOGICAL_SHIFT_LEFT_IMMS());
            case 0x1B1:
                return ARM_MOVS(ARM_LOGICAL_SHIFT_LEFT_REGS());
            case 0x1B2: case 0x1BA:
                return ARM_MOVS(ARM_LOGICAL_SHIFT_RIGHT_IMMS());
            case 0x1B3:
                return ARM_MOVS(ARM_LOGICAL_SHIFT_RIGHT_REGS());
            case 0x1B4: case 0x1BC:
                return ARM_MOVS(ARM_ARITHMETIC_SHIFT_RIGHT_IMMS());
            case 0x1B5:
                return ARM_MOVS(ARM_ARITHMETIC_SHIFT_RIGHT_REGS());
            case 0x1B6: case 0x1BE:
                return ARM_MOVS(ARM_ROTATE_RIGHT_IMMS());
            case 0x1BB:
                return ARM_LDRH_PRE_WRITEBACK(ARM_HALFWORD_SIGNED_DATA_TRANSFER_REG());
            case 0x1BD:
                return ARM_LDRSB_PRE_WRITEBACK(ARM_HALFWORD_SIGNED_DATA_TRANSFER_REG());
            case 0x1C0: case 0x1C8:
                return ARM_BIC(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x1CB:
                return ARM_STRH_PRE(ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM());
            case 0x1D0: case 0x1D8:
                return ARM_BICS(ARM_LOGICAL_SHIFT_LEFT_IMMS());
            case 0x1D3:
                return ARM_BICS(ARM_LOGICAL_SHIFT_RIGHT_REGS());
            case 0x1D4: case 0x1DC:
                return ARM_BICS(ARM_ARITHMETIC_SHIFT_RIGHT_IMMS());
            case 0x1DB:
                return ARM_LDRH_PRE(ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM());
            case 0x1DD:
                return ARM_LDRSB_PRE(ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM());
            case 0x1DF:
                return ARM_LDRSH_PRE(ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM());
            case 0x1E0: case 0x1E8:
                return ARM_MVN(ARM_LOGICAL_SHIFT_LEFT_IMM());
            case 0x1E1:
                return ARM_MVN(ARM_LOGICAL_SHIFT_LEFT_REG());
            case 0x1E2: case 0x1EA:
                return ARM_MVN(ARM_LOGICAL_SHIFT_RIGHT_IMM());
            case 0x1E4: case 0x1EC:
                return ARM_MVN(ARM_ARITHMETIC_SHIFT_RIGHT_IMM());
            case 0x1F0: case 0x1F8:
                return ARM_MVNS(ARM_LOGICAL_SHIFT_LEFT_IMMS());
            case 0x1F4: case 0x1FC:
                return ARM_MVNS(ARM_ARITHMETIC_SHIFT_RIGHT_IMMS());
            case 0x1FB:
                return ARM_LDRH_PRE_WRITEBACK(ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM());
            case 0x1FD:
                return ARM_LDRSB_PRE_WRITEBACK(ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM()); 
            case 0x1FF:
                return ARM_LDRSH_PRE_WRITEBACK(ARM_HALFWORD_SIGNED_DATA_TRANSFER_IMM());
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
            case 0x230: case 0x231: case 0x232: case 0x233: 
            case 0x234: case 0x235: case 0x236: case 0x237: 
            case 0x238: case 0x239: case 0x23A: case 0x23B: 
            case 0x23C: case 0x23D: case 0x23E: case 0x23F:
                return ARM_EORS(ARM_DATA_PROCESSING_IMMS());
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
            case 0x260: case 0x261: case 0x262: case 0x263: 
            case 0x264: case 0x265: case 0x266: case 0x267: 
            case 0x268: case 0x269: case 0x26A: case 0x26B: 
            case 0x26C: case 0x26D: case 0x26E: case 0x26F:
                return ARM_RSB(ARM_DATA_PROCESSING_IMM());
            case 0x270: case 0x271: case 0x272: case 0x273: 
            case 0x274: case 0x275: case 0x276: case 0x277: 
            case 0x278: case 0x279: case 0x27A: case 0x27B: 
            case 0x27C: case 0x27D: case 0x27E: case 0x27F:
                return ARM_RSBS(ARM_DATA_PROCESSING_IMM());
            case 0x280: case 0x281: case 0x282: case 0x283: 
            case 0x284: case 0x285: case 0x286: case 0x287: 
            case 0x288: case 0x289: case 0x28A: case 0x28B: 
            case 0x28C: case 0x28D: case 0x28E: case 0x28F:
                return ARM_ADD(ARM_DATA_PROCESSING_IMM());
            case 0x290: case 0x291: case 0x292: case 0x293: 
            case 0x294: case 0x295: case 0x296: case 0x297: 
            case 0x298: case 0x299: case 0x29A: case 0x29B: 
            case 0x29C: case 0x29D: case 0x29E: case 0x29F:
                return ARM_ADDS(ARM_DATA_PROCESSING_IMM());
            case 0x2A0: case 0x2A1: case 0x2A2: case 0x2A3: 
            case 0x2A4: case 0x2A5: case 0x2A6: case 0x2A7: 
            case 0x2A8: case 0x2A9: case 0x2AA: case 0x2AB: 
            case 0x2AC: case 0x2AD: case 0x2AE: case 0x2AF:
                return ARM_ADC(ARM_DATA_PROCESSING_IMM());
            case 0x2B0: case 0x2B1: case 0x2B2: case 0x2B3: 
            case 0x2B4: case 0x2B5: case 0x2B6: case 0x2B7: 
            case 0x2B8: case 0x2B9: case 0x2BA: case 0x2BB: 
            case 0x2BC: case 0x2BD: case 0x2BE: case 0x2BF:
                return ARM_ADCS(ARM_DATA_PROCESSING_IMM());
            case 0x2C0: case 0x2C1: case 0x2C2: case 0x2C3: 
            case 0x2C4: case 0x2C5: case 0x2C6: case 0x2C7: 
            case 0x2C8: case 0x2C9: case 0x2CA: case 0x2CB: 
            case 0x2CC: case 0x2CD: case 0x2CE: case 0x2CF:
                return ARM_SBC(ARM_DATA_PROCESSING_IMM());
            case 0x2D0: case 0x2D1: case 0x2D2: case 0x2D3: 
            case 0x2D4: case 0x2D5: case 0x2D6: case 0x2D7: 
            case 0x2D8: case 0x2D9: case 0x2DA: case 0x2DB: 
            case 0x2DC: case 0x2DD: case 0x2DE: case 0x2DF:
                return ARM_SBCS(ARM_DATA_PROCESSING_IMM());
            case 0x2E0: case 0x2E1: case 0x2E2: case 0x2E3: 
            case 0x2E4: case 0x2E5: case 0x2E6: case 0x2E7: 
            case 0x2E8: case 0x2E9: case 0x2EA: case 0x2EB: 
            case 0x2EC: case 0x2ED: case 0x2EE: case 0x2EF:
                return ARM_RSC(ARM_DATA_PROCESSING_IMM());
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
            case 0x330: case 0x331: case 0x332: case 0x333: 
            case 0x334: case 0x335: case 0x336: case 0x337: 
            case 0x338: case 0x339: case 0x33A: case 0x33B: 
            case 0x33C: case 0x33D: case 0x33E: case 0x33F:
                return ARM_TEQS(ARM_DATA_PROCESSING_IMMS());
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
            case 0x390: case 0x391: case 0x392: case 0x393: 
            case 0x394: case 0x395: case 0x396: case 0x397: 
            case 0x398: case 0x399: case 0x39A: case 0x39B: 
            case 0x39C: case 0x39D: case 0x39E: case 0x39F:
                return ARM_ORRS(ARM_DATA_PROCESSING_IMMS());
            case 0x3A0: case 0x3A1: case 0x3A2: case 0x3A3: 
            case 0x3A4: case 0x3A5: case 0x3A6: case 0x3A7: 
            case 0x3A8: case 0x3A9: case 0x3AA: case 0x3AB: 
            case 0x3AC: case 0x3AD: case 0x3AE: case 0x3AF:
                return ARM_MOV(ARM_DATA_PROCESSING_IMM());
            case 0x3B0: case 0x3B1: case 0x3B2: case 0x3B3: 
            case 0x3B4: case 0x3B5: case 0x3B6: case 0x3B7: 
            case 0x3B8: case 0x3B9: case 0x3BA: case 0x3BB: 
            case 0x3BC: case 0x3BD: case 0x3BE: case 0x3BF:
                return ARM_MOVS(ARM_DATA_PROCESSING_IMMS());
            case 0x3C0: case 0x3C1: case 0x3C2: case 0x3C3: 
            case 0x3C4: case 0x3C5: case 0x3C6: case 0x3C7: 
            case 0x3C8: case 0x3C9: case 0x3CA: case 0x3CB: 
            case 0x3CC: case 0x3CD: case 0x3CE: case 0x3CF:
                return ARM_BIC(ARM_DATA_PROCESSING_IMM());
            case 0x3D0: case 0x3D1: case 0x3D2: case 0x3D3: 
            case 0x3D4: case 0x3D5: case 0x3D6: case 0x3D7: 
            case 0x3D8: case 0x3D9: case 0x3DA: case 0x3DB: 
            case 0x3DC: case 0x3DD: case 0x3DE: case 0x3DF:
                return ARM_BICS(ARM_DATA_PROCESSING_IMMS());
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
            case 0x410: case 0x411: case 0x412: case 0x413: 
            case 0x414: case 0x415: case 0x416: case 0x417: 
            case 0x418: case 0x419: case 0x41A: case 0x41B: 
            case 0x41C: case 0x41D: case 0x41E: case 0x41F:
                return ARM_LDR_POST(-ARM_SINGLE_DATA_TRANSFER_IMM());
            case 0x440: case 0x441: case 0x442: case 0x443: 
            case 0x444: case 0x445: case 0x446: case 0x447: 
            case 0x448: case 0x449: case 0x44A: case 0x44B: 
            case 0x44C: case 0x44D: case 0x44E: case 0x44F:
                return ARM_STRB_POST(-ARM_SINGLE_DATA_TRANSFER_IMM());
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
            case 0x530: case 0x531: case 0x532: case 0x533:
            case 0x534: case 0x535: case 0x536: case 0x537:
            case 0x538: case 0x539: case 0x53A: case 0x53B:
            case 0x53C: case 0x53D: case 0x53E: case 0x53F:
                return ARM_LDR_PRE_WRITEBACK(-ARM_SINGLE_DATA_TRANSFER_IMM());
            case 0x540: case 0x541: case 0x542: case 0x543:
            case 0x544: case 0x545: case 0x546: case 0x547:
            case 0x548: case 0x549: case 0x54A: case 0x54B:
            case 0x54C: case 0x54D: case 0x54E: case 0x54F:
                return ARM_STRB_PRE(-ARM_SINGLE_DATA_TRANSFER_IMM());
            case 0x550: case 0x551: case 0x552: case 0x553:
            case 0x554: case 0x555: case 0x556: case 0x557:
            case 0x558: case 0x559: case 0x55A: case 0x55B:
            case 0x55C: case 0x55D: case 0x55E: case 0x55F:
                return ARM_LDRB_PRE(-ARM_SINGLE_DATA_TRANSFER_IMM());
            case 0x560: case 0x561: case 0x562: case 0x563:
            case 0x564: case 0x565: case 0x566: case 0x567:
            case 0x568: case 0x569: case 0x56A: case 0x56B:
            case 0x56C: case 0x56D: case 0x56E: case 0x56F:
                return ARM_STRB_PRE_WRITEBACK(-ARM_SINGLE_DATA_TRANSFER_IMM());
            case 0x570: case 0x571: case 0x572: case 0x573:
            case 0x574: case 0x575: case 0x576: case 0x577:
            case 0x578: case 0x579: case 0x57A: case 0x57B:
            case 0x57C: case 0x57D: case 0x57E: case 0x57F:
                return ARM_LDRB_PRE_WRITEBACK(-ARM_SINGLE_DATA_TRANSFER_IMM());
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
            case 0x5A0: case 0x5A1: case 0x5A2: case 0x5A3:
            case 0x5A4: case 0x5A5: case 0x5A6: case 0x5A7:
            case 0x5A8: case 0x5A9: case 0x5AA: case 0x5AB:
            case 0x5AC: case 0x5AD: case 0x5AE: case 0x5AF:
                return ARM_STR_PRE_WRITEBACK(ARM_SINGLE_DATA_TRANSFER_IMM());
            case 0x5B0: case 0x5B1: case 0x5B2: case 0x5B3:
            case 0x5B4: case 0x5B5: case 0x5B6: case 0x5B7:
            case 0x5B8: case 0x5B9: case 0x5BA: case 0x5BB:
            case 0x5BC: case 0x5BD: case 0x5BE: case 0x5BF:
                return ARM_LDR_PRE_WRITEBACK(ARM_SINGLE_DATA_TRANSFER_IMM());
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
            case 0x5E0: case 0x5E1: case 0x5E2: case 0x5E3: 
            case 0x5E4: case 0x5E5: case 0x5E6: case 0x5E7: 
            case 0x5E8: case 0x5E9: case 0x5EA: case 0x5EB: 
            case 0x5EC: case 0x5ED: case 0x5EE: case 0x5EF:
                return ARM_STRB_PRE_WRITEBACK(ARM_SINGLE_DATA_TRANSFER_IMM());
            case 0x5F0: case 0x5F1: case 0x5F2: case 0x5F3:
            case 0x5F4: case 0x5F5: case 0x5F6: case 0x5F7:
            case 0x5F8: case 0x5F9: case 0x5FA: case 0x5FB:
            case 0x5FC: case 0x5FD: case 0x5FE: case 0x5FF:
                return ARM_LDRB_PRE_WRITEBACK(ARM_SINGLE_DATA_TRANSFER_IMM());
            case 0x610: case 0x618:
                return ARM_LDR_POST(-ARM_RPLL());
            case 0x612: case 0x61A:
                return ARM_LDR_POST(-ARM_RPLR());
            case 0x614: case 0x61C:
                return ARM_LDR_POST(-ARM_RPAR());
            case 0x616: case 0x61E:
                return ARM_LDR_POST(-ARM_RPRR());
            case 0x680: case 0x688:
                return ARM_STR_POST(ARM_RPLL());
            case 0x690: case 0x698:
                return ARM_LDR_POST(ARM_RPLL());
            case 0x692: case 0x69A:
                return ARM_LDR_POST(ARM_RPLR());
            case 0x694: case 0x69C:
                return ARM_LDR_POST(ARM_RPAR());
            case 0x696: case 0x69E:
                return ARM_LDR_POST(ARM_RPRR());
            case 0x700: case 0x708:
                return ARM_STR_PRE(-ARM_RPLL());
            case 0x710: case 0x718:
                return ARM_LDR_PRE(-ARM_RPLL());
            case 0x712: case 0x71A:
                return ARM_LDR_PRE(-ARM_RPLR());
            case 0x714: case 0x71C:
                return ARM_LDR_PRE(-ARM_RPAR());
            case 0x716: case 0x71E:
                return ARM_LDR_PRE(-ARM_RPRR());
            case 0x730: case 0x738:
                return ARM_LDR_PRE_WRITEBACK(-ARM_RPLL());
            case 0x732: case 0x73A:
                return ARM_LDR_PRE_WRITEBACK(-ARM_RPLR());
            case 0x734: case 0x73C:
                return ARM_LDR_PRE_WRITEBACK(-ARM_RPAR());
            case 0x736: case 0x73E:
                return ARM_LDR_PRE_WRITEBACK(-ARM_RPRR());
            case 0x740: case 0x748:
                return ARM_STRB_PRE(-ARM_RPLL());
            case 0x750: case 0x758:
                return ARM_LDRB_PRE(-ARM_RPLL());
            case 0x770: case 0x778:
                return ARM_LDRB_PRE_WRITEBACK(-ARM_RPLL());
            case 0x780: case 0x788:
                return ARM_STR_PRE(ARM_RPLL());
            case 0x790: case 0x798:
                return ARM_LDR_PRE(ARM_RPLL());
            case 0x792: case 0x79A:
                return ARM_LDR_PRE(ARM_RPLR());
            case 0x794: case 0x79C:
                return ARM_LDR_PRE(ARM_RPAR());
            case 0x796: case 0x79E:
                return ARM_LDR_PRE(ARM_RPRR());
            case 0x7A0: case 0x7A8:
                return ARM_STR_PRE_WRITEBACK(ARM_RPLL());
            case 0x7B0: case 0x7B8:
                return ARM_LDR_PRE_WRITEBACK(ARM_RPLL());
            case 0x7B2: case 0x7BA:
                return ARM_LDR_PRE_WRITEBACK(ARM_RPLR());
            case 0x7B4: case 0x7BC:
                return ARM_LDR_PRE_WRITEBACK(ARM_RPAR());
            case 0x7B6: case 0x7BE:
                return ARM_LDR_PRE_WRITEBACK(ARM_RPRR());
            case 0x7C0: case 0x7C8:
                return ARM_STRB_PRE(ARM_RPLL());
            case 0x7C2: case 0x7CA:
                return ARM_STRB_PRE(ARM_RPLR());
            case 0x7C4: case 0x7CC:
                return ARM_STRB_PRE(ARM_RPAR());
            case 0x7D0: case 0x7D8:
                return ARM_LDRB_PRE(ARM_RPLL());
            case 0x7D2: case 0x7DA:
                return ARM_LDRB_PRE(ARM_RPLR());
            case 0x7D4: case 0x7DC:
                return ARM_LDRB_PRE(ARM_RPAR());
            case 0x7F0: case 0x7F8:
                return ARM_LDRB_PRE_WRITEBACK(ARM_RPLL());
            case 0x820: case 0x821: case 0x822: case 0x823:
            case 0x824: case 0x825: case 0x826: case 0x827:
            case 0x828: case 0x829: case 0x82A: case 0x82B:
            case 0x82C: case 0x82D: case 0x82E: case 0x82F:
                return ARM_STM_DECREMENT_AFTER_WRITEBACK();
            case 0x830: case 0x831: case 0x832: case 0x833:
            case 0x834: case 0x835: case 0x836: case 0x837:
            case 0x838: case 0x839: case 0x83A: case 0x83B:
            case 0x83C: case 0x83D: case 0x83E: case 0x83F:
                return ARM_LDM_DECREMENT_AFTER_WRITEBACK();
            case 0x870: case 0x871: case 0x872: case 0x873:
            case 0x874: case 0x875: case 0x876: case 0x877:
            case 0x878: case 0x879: case 0x87A: case 0x87B:
            case 0x87C: case 0x87D: case 0x87E: case 0x87F:
                return ARM_LDM_DECREMENT_AFTER_USER_WRITEBACK();
            case 0x880: case 0x881: case 0x882: case 0x883:
            case 0x884: case 0x885: case 0x886: case 0x887:
            case 0x888: case 0x889: case 0x88A: case 0x88B:
            case 0x88C: case 0x88D: case 0x88E: case 0x88F:
                return ARM_STM_INCREMENT_AFTER();
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
            case 0x8D0: case 0x8D1: case 0x8D2: case 0x8D3:
            case 0x8D4: case 0x8D5: case 0x8D6: case 0x8D7:
            case 0x8D8: case 0x8D9: case 0x8DA: case 0x8DB:
            case 0x8DC: case 0x8DD: case 0x8DE: case 0x8DF:
                return ARM_LDM_INCREMENT_AFTER_USER();
            case 0x8F0: case 0x8F1: case 0x8F2: case 0x8F3:
            case 0x8F4: case 0x8F5: case 0x8F6: case 0x8F7:
            case 0x8F8: case 0x8F9: case 0x8FA: case 0x8FB:
            case 0x8FC: case 0x8FD: case 0x8FE: case 0x8FF:
                return ARM_LDM_INCREMENT_AFTER_USER_WRITEBACK();
            case 0x900: case 0x901: case 0x902: case 0x903:
            case 0x904: case 0x905: case 0x906: case 0x907:
            case 0x908: case 0x909: case 0x90A: case 0x90B:
            case 0x90C: case 0x90D: case 0x90E: case 0x90F:
                return ARM_STM_DECREMENT_BEFORE();
            case 0x910: case 0x911: case 0x912: case 0x913:
            case 0x914: case 0x915: case 0x916: case 0x917:
            case 0x918: case 0x919: case 0x91A: case 0x91B:
            case 0x91C: case 0x91D: case 0x91E: case 0x91F:
                return ARM_LDM_DECREMENT_BEFORE();
            case 0x920: case 0x921: case 0x922: case 0x923:
            case 0x924: case 0x925: case 0x926: case 0x927:
            case 0x928: case 0x929: case 0x92A: case 0x92B:
            case 0x92C: case 0x92D: case 0x92E: case 0x92F:
                return ARM_STM_DECREMENT_BEFORE_WRITEBACK();
            case 0x930: case 0x931: case 0x932: case 0x933:
            case 0x934: case 0x935: case 0x936: case 0x937:
            case 0x938: case 0x939: case 0x93A: case 0x93B:
            case 0x93C: case 0x93D: case 0x93E: case 0x93F:
                return ARM_LDM_DECREMENT_BEFORE_WRITEBACK();
            case 0x970: case 0x971: case 0x972: case 0x973:
            case 0x974: case 0x975: case 0x976: case 0x977:
            case 0x978: case 0x979: case 0x97A: case 0x97B:
            case 0x97C: case 0x97D: case 0x97E: case 0x97F:
                return ARM_LDM_DECREMENT_BEFORE_USER_WRITEBACK();
            case 0x980: case 0x981: case 0x982: case 0x983:
            case 0x984: case 0x985: case 0x986: case 0x987:
            case 0x988: case 0x989: case 0x98A: case 0x98B:
            case 0x98C: case 0x98D: case 0x98E: case 0x98F:
                return ARM_STM_INCREMENT_BEFORE();
            case 0x990: case 0x991: case 0x992: case 0x993:
            case 0x994: case 0x995: case 0x996: case 0x997:
            case 0x998: case 0x999: case 0x99A: case 0x99B:
            case 0x99C: case 0x99D: case 0x99E: case 0x99F:
                return ARM_LDM_INCREMENT_BEFORE();
            case 0x9A0: case 0x9A1: case 0x9A2: case 0x9A3:
            case 0x9A4: case 0x9A5: case 0x9A6: case 0x9A7:
            case 0x9A8: case 0x9A9: case 0x9AA: case 0x9AB:
            case 0x9AC: case 0x9AD: case 0x9AE: case 0x9AF:
                return ARM_STM_INCREMENT_BEFORE_WRITEBACK();
            case 0x9B0: case 0x9B1: case 0x9B2: case 0x9B3:
            case 0x9B4: case 0x9B5: case 0x9B6: case 0x9B7:
            case 0x9B8: case 0x9B9: case 0x9BA: case 0x9BB:
            case 0x9BC: case 0x9BD: case 0x9BE: case 0x9BF:
                return ARM_LDM_INCREMENT_BEFORE_WRITEBACK();
            case 0x9C0: case 0x9C1: case 0x9C2: case 0x9C3:
            case 0x9C4: case 0x9C5: case 0x9C6: case 0x9C7:
            case 0x9C8: case 0x9C9: case 0x9CA: case 0x9CB:
            case 0x9CC: case 0x9CD: case 0x9CE: case 0x9CF:
                return ARM_STM_INCREMENT_BEFORE_USER();
            case 0x9D0: case 0x9D1: case 0x9D2: case 0x9D3:
            case 0x9D4: case 0x9D5: case 0x9D6: case 0x9D7:
            case 0x9D8: case 0x9D9: case 0x9DA: case 0x9DB:
            case 0x9DC: case 0x9DD: case 0x9DE: case 0x9DF:
                return ARM_LDM_INCREMENT_BEFORE_USER();
            case 0x9E0: case 0x9E1: case 0x9E2: case 0x9E3:
            case 0x9E4: case 0x9E5: case 0x9E6: case 0x9E7:
            case 0x9E8: case 0x9E9: case 0x9EA: case 0x9EB:
            case 0x9EC: case 0x9ED: case 0x9EE: case 0x9EF:
                return ARM_STM_INCREMENT_BEFORE_USER_WRITEBACK();
            case 0x9F0: case 0x9F1: case 0x9F2: case 0x9F3:
            case 0x9F4: case 0x9F5: case 0x9F6: case 0x9F7:
            case 0x9F8: case 0x9F9: case 0x9FA: case 0x9FB:
            case 0x9FC: case 0x9FD: case 0x9FE: case 0x9FF:
                return ARM_LDM_INCREMENT_BEFORE_USER_WRITEBACK();
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
                    return ARM_BLX();
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
                // check bits 28..31
                // if it is 0b1111 then its a blx_offset
                if ((instruction & 0xF0000000) != 0xF0000000) {
                    return ARM_BL();
                } else {
                    return ARM_BLX();
                }
                return;
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
                log_fatal("[[Interpreter]] Undefined ARM instruction %08x with index %03x", instruction, index);
            }
        } else {
            regs.r[15] += 4;
        }
    } else {
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
                return THUMB_EOR_DATA_PROCESSING();
            case 2:
                return THUMB_LSL_DATA_PROCESSING();
            case 3:
                return THUMB_LSR_DATA_PROCESSING();
            default:
                log_fatal("[Interpreter] instruction 0x%04x with index 0x%02x is unimplemented!", instruction, index);
            }
            break;
        case 0x41:
            // using the data processing opcode table with bits
            switch ((instruction >> 6) & 0x3) {
            case 0:
                return THUMB_ASR_DATA_PROCESSING();
            case 1:
                return THUMB_ADC_DATA_PROCESSING();
            case 2:
                return THUMB_SBC_DATA_PROCESSING();
            case 3:
                return THUMB_ROR_DATA_PROCESSING();
            default:
                log_fatal("[Interpreter] instruction 0x%04x with index 0x%02x is unimplemented!", instruction, index);
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
                log_fatal("[Interpreter] instruction 0x%04x with index 0x%02x is unimplemented!", instruction, index);
            }
            break;
        case 0x43:
            // using the data processing opcode table with bits
            switch ((instruction >> 6) & 0x3) {
            case 0:
                return THUMB_ORR_DATA_PROCESSING();
            case 1:
                return THUMB_MUL_DATA_PROCESSING();
            case 2:
                return THUMB_BIC_DATA_PROCESSING();
            case 3:
                return THUMB_MVN_DATA_PROCESSING();
            default:
                log_fatal("[Interpreter] instruction 0x%04x with index 0x%02x is unimplemented!", instruction, index);
            }
            break;
        case 0x44:
            return THUMB_ADDH();
        case 0x45:
            return THUMB_CMPH();
        case 0x46:
            return THUMB_MOVH();
        case 0x47:
            if (instruction & (1 << 7)) {
                return THUMB_BLX_REG();
            } else {
                return THUMB_BX_REG();
            }
            return;
        case 0x48: case 0x49: case 0x4A: case 0x4B: 
        case 0x4C: case 0x4D: case 0x4E: case 0x4F:
            return THUMB_LDR_PC();
        case 0x50: case 0x51:
            return THUMB_STR_REG();
        case 0x52: case 0x53:
            return THUMB_STRH_REG();
        case 0x54: case 0x55:
            return THUMB_STRB_REG();
        case 0x56: case 0x57:
            return THUMB_LDRSB_REG();
        case 0x58: case 0x59:
            return THUMB_LDR_REG();
        case 0x5A: case 0x5B:
            return THUMB_LDRH_REG();
        case 0x5C: case 0x5D:
            return THUMB_LDRB_REG();
        case 0x5E: case 0x5F:
            return THUMB_LDRSH_REG();
        case 0x60: case 0x61: case 0x62: case 0x63: 
        case 0x64: case 0x65: case 0x66: case 0x67:
            return THUMB_STR_IMM5();
        case 0x68: case 0x69: case 0x6A: case 0x6B: 
        case 0x6C: case 0x6D: case 0x6E: case 0x6F:
            return THUMB_LDR_IMM5();
        case 0x70: case 0x71: case 0x72: case 0x73: 
        case 0x74: case 0x75: case 0x76: case 0x77:
            return THUMB_STRB_IMM5();
        case 0x78: case 0x79: case 0x7A: case 0x7B: 
        case 0x7C: case 0x7D: case 0x7E: case 0x7F:
            return THUMB_LDRB_IMM5();
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
        case 0xA0: case 0xA1: case 0xA2: case 0xA3: 
        case 0xA4: case 0xA5: case 0xA6: case 0xA7:
            return THUMB_ADD_PC_REG();
        case 0xA8: case 0xA9: case 0xAA: case 0xAB: 
        case 0xAC: case 0xAD: case 0xAE: case 0xAF:
            return THUMB_ADD_SP_REG();
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
        case 0xC0: case 0xC1: case 0xC2: case 0xC3: 
        case 0xC4: case 0xC5: case 0xC6: case 0xC7:
            return THUMB_STMIA_REG();
        case 0xC8: case 0xC9: case 0xCA: case 0xCB: 
        case 0xCC: case 0xCD: case 0xCE: case 0xCF:
            return THUMB_LDMIA_REG();
        case 0xD0:
            return THUMB_BEQ();
        case 0xD1:
            return THUMB_BNE();
        case 0xD2:
            return THUMB_BCS();
        case 0xD3:
            return THUMB_BCC();
        case 0xD4:
            return THUMB_BMI();
        case 0xD5:
            return THUMB_BPL();
        case 0xD8:
            return THUMB_BHI();
        case 0xD9:
            return THUMB_BLS();
        case 0xDA:
            return THUMB_BGE();
        case 0xDB:
            return THUMB_BLT();
        case 0xDC:
            return THUMB_BGT();
        case 0xDD:
            return THUMB_BLE();
        case 0xDF:
            return THUMB_SWI();
        case 0xE0: case 0xE1: case 0xE2: case 0xE3: 
        case 0xE4: case 0xE5: case 0xE6: case 0xE7:
            return THUMB_B();
        case 0xE8: case 0xE9: case 0xEA: case 0xEB: 
        case 0xEC: case 0xED: case 0xEE: case 0xEF:
            return THUMB_BLX_OFFSET(); 
        case 0xF0: case 0xF1: case 0xF2: case 0xF3: 
        case 0xF4: case 0xF5: case 0xF6: case 0xF7:
            return THUMB_BL_SETUP();
        case 0xF8: case 0xF9: case 0xFA: case 0xFB: 
        case 0xFC: case 0xFD: case 0xFE: case 0xFF:
            return THUMB_BL_OFFSET();
        default:
            log_fatal("[Interpreter] instruction 0x%04x with index 0x%02x is unimplemented!", instruction, index);
        }
    }
}

void Interpreter::ARM_MRC() {
    // armv5 exclusive as it involves coprocessor transfers
    u8 cp = (instruction >> 8) & 0xF;
    u8 crm = instruction & 0xF;
    u8 crn = (instruction >> 16) & 0xF;
    u8 opcode2 = (instruction >> 5) & 0x7;
    u8 rd = (instruction >> 12) & 0xF;

    if (arch == CPUArch::ARMv4) {
        if (cp == 15) {
            // generate an undefined exception
            return ARM_UND();
        } else {
            log_warn("arm7 cp%d, c%d, c%d, c%d", cp, crn, crm, opcode2);
            return;
        }
    }

    u32 data = cp15->Read(crn, crm, opcode2);

    if (rd == 15) {
        // set flags instead
        regs.cpsr = (data & 0xF0000000) | (regs.cpsr & 0x0FFFFFFF);
    } else {
        // set rd normally
        regs.r[rd] = data;
    }

    regs.r[15] += 4;
}

void Interpreter::ARM_MCR() {
    // armv5 exclusive as it involves coprocessor transfers
    if (arch == CPUArch::ARMv4) {
        return;
    }

    u8 crm = instruction & 0xF;
    u8 crn = (instruction >> 16) & 0xF;
    u8 opcode2 = (instruction >> 5) & 0x7;
    u8 rd = (instruction >> 12) & 0xF;
    cp15->Write(crn, crm, opcode2, regs.r[rd]);

    regs.r[15] += 4;
}

void Interpreter::ARM_SWI() {
    // store the cpsr in spsr_svc
    regs.spsr_banked[BANK_SVC] = regs.cpsr;

    // enter supervisor mode
    SwitchMode(SVC);

    // disable normal interrupts
    regs.cpsr |= (1 << 7);

    regs.r[14] = regs.r[15] - 4;
    // check the exception base and jump to the correct address in the bios
    // also only use cp15 exception base from control register if arm9
    regs.r[15] = ((arch == CPUArch::ARMv5) ? cp15->GetExceptionBase() : 0x00000000) + 0x08;
    
    ARMFlushPipeline();
}

void Interpreter::THUMB_SWI() {
    // store the cpsr in spsr_svc
    regs.spsr_banked[BANK_SVC] = regs.cpsr;

    // enter supervisor mode
    SwitchMode(SVC);

    // always execute in arm state
    regs.cpsr &= ~(1 << 5);

    // disable normal interrupts
    regs.cpsr |= (1 << 7);
    
    regs.r[14] = regs.r[15] - 2;
    // check the exception base and jump to the correct address in the bios
    regs.r[15] = ((arch == CPUArch::ARMv5) ? cp15->GetExceptionBase() : 0x00000000) + 0x08;
    
    ARMFlushPipeline();
}

void Interpreter::ARM_UND() {
    // store the cpsr in spsr_und
    regs.spsr_banked[BANK_UND] = regs.cpsr;

    // enter undefined mode
    SwitchMode(UND);

    // disable normal interrupts
    regs.cpsr |= (1 << 7);

    regs.r[14] = regs.r[15] - 4;
    // check the exception base and jump to the correct address in the bios
    // also only use cp15 exception base from control register if arm9
    regs.r[15] = ((arch == CPUArch::ARMv5) ? cp15->GetExceptionBase() : 0x00000000) + 0x04;
    
    ARMFlushPipeline();
}