#include <core/arm/interpreter/interpreter.h>

Interpreter::Interpreter(MemoryBase& memory, CPUArch arch) : CPUBase(memory, arch) {
    GenerateARMTable();
    GenerateThumbTable();
    GenerateConditionTable();
}

Interpreter::~Interpreter() {

}

void Interpreter::Reset() {

}

void Interpreter::Run(int cycles) {
    while (cycles--) {
        if (halted) {
            return;
        }

        // stepping the pipeline must happen before an instruction is executed incase the instruction is a branch which would flush and then step the pipeline (not correct)
        instruction = pipeline[0]; // store the current executing instruction 

        printf("instruction is %08x at %08x\n", instruction, regs.r[15]);
        // shift the pipeline
        pipeline[0] = pipeline[1];
        // fill the 2nd item with the new instruction to be read
        // TODO: align r15
        if (IsARM()) {
            pipeline[1] = ReadWord(regs.r[15]);
            
            u32 index = ((instruction >> 16) & 0xFF0) | ((instruction >> 4) & 0xF);

            if (ConditionEvaluate(instruction >> 28)) {
                (this->*arm_lut[index])();
            } else {
                regs.r[15] += 4;
            }
        } else {
            pipeline[1] = ReadHalf(regs.r[15]);
            u8 index = (instruction >> 8) & 0xFF;

            (this->*thumb_lut[index])();
        }
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

void Interpreter::UnimplementedInstruction() {
    log_fatal("[Interpreter] Instruction %08x is unimplemented at r15 = %08x", instruction, regs.r[15]);
}

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