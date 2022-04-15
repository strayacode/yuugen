#include <algorithm>
#include "core/arm/cpu_core.h"
#include "core/hw/cp15/cp15.h"

static Decoder<CPUCore> decoder;

CPUCore::CPUCore(MemoryBase& memory, CPUArch arch, CP15* cp15) : memory(memory), arch(arch), cp15(cp15) {
    GenerateConditionTable();
}

void CPUCore::Reset() {
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

    halted = false;

    // enter supervisor mode
    regs.cpsr = 0xD3;
    SwitchMode(SVC);

    pipeline[0] = pipeline[1] = 0;
    ime = 0;
    ie = 0;
    irf = 0;
    instruction = 0;
}

void CPUCore::RunInterpreter(int cycles) {
    while (cycles-- > 0) {
        if (halted) {
            return;
        }

        if (ime && (ie & irf) && !(regs.cpsr & (1 << 7))) {
            HandleInterrupt();
        }

        // store the current executing instruction
        instruction = pipeline[0];
        
        // shift the pipeline
        pipeline[0] = pipeline[1];

        // TODO: align r15
        if (IsARM()) {
            pipeline[1] = ReadWord(regs.r[15]);
        } else {
            pipeline[1] = ReadHalf(regs.r[15]);
        }

        if (IsARM()) {
            Instruction inst = decoder.decode_arm(instruction);

            if (ConditionEvaluate(instruction >> 28)) {
                (this->*inst)();
            } else {
                regs.r[15] += 4;
            }
        } else {
            Instruction inst = decoder.decode_thumb(instruction);
            (this->*inst)();
        }
    }
}

void CPUCore::DirectBoot(u32 entrypoint) {
    regs.r[12] = regs.r[14] = regs.r[15] = entrypoint;

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

void CPUCore::FirmwareBoot() {
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
    
void CPUCore::SendInterrupt(InterruptType interrupt_type) {
    // set the appropriate bit in IF
    irf |= (1 << static_cast<int>(interrupt_type));
    
    // check if the interrupt is enabled too
    if (ie & (1 << static_cast<int>(interrupt_type))) {
        // to unhalt on the arm9 ime needs to be set
        if (ime || arch == CPUArch::ARMv4) {
            halted = false;
        }
    }
}

void CPUCore::Halt() {
    halted = true;
}

bool CPUCore::Halted() {
    return halted;
}

u8 CPUCore::ReadByte(u32 addr) {
    return memory.FastRead<u8>(addr);
}

u16 CPUCore::ReadHalf(u32 addr) {
    return memory.FastRead<u16>(addr);
}

u32 CPUCore::ReadWord(u32 addr) {
    return memory.FastRead<u32>(addr);
}

u32 CPUCore::ReadWordRotate(u32 addr) {
    u32 return_value = memory.FastRead<u32>(addr);

    if (addr & 0x3) {
        int shift_amount = (addr & 0x3) * 8;
        return_value = rotate_right(return_value, shift_amount);
    }

    return return_value;
}

void CPUCore::WriteByte(u32 addr, u8 data) {
    memory.FastWrite<u8>(addr, data);
}

void CPUCore::WriteHalf(u32 addr, u16 data) {
    memory.FastWrite<u16>(addr, data);
}

void CPUCore::WriteWord(u32 addr, u32 data) {
    memory.FastWrite<u32>(addr, data);
}

void CPUCore::ARMFlushPipeline() {
    regs.r[15] &= ~3;
    pipeline[0] = ReadWord(regs.r[15]);
    pipeline[1] = ReadWord(regs.r[15] + 4);
    regs.r[15] += 8;
}

void CPUCore::ThumbFlushPipeline() {
    regs.r[15] &= ~1;
    pipeline[0] = ReadHalf(regs.r[15]);
    pipeline[1] = ReadHalf(regs.r[15] + 2);
    regs.r[15] += 4;
}

void CPUCore::GenerateConditionTable() {
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

bool CPUCore::ConditionEvaluate(u8 condition) {
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

bool CPUCore::GetConditionFlag(int condition_flag) {
    return (regs.cpsr & (1 << condition_flag));
}

void CPUCore::SetConditionFlag(int condition_flag, int value) {
    if (value) {
        regs.cpsr |= (1 << condition_flag);
    } else {
        regs.cpsr &= ~(1 << condition_flag);
    }
}

u32 CPUCore::GetCurrentSPSR() {
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

void CPUCore::SetCurrentSPSR(u32 data) {
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

bool CPUCore::HasSPSR() {
    u8 current_mode = regs.cpsr & 0x1F;

    return (current_mode != USR && current_mode != SYS);
}

bool CPUCore::PrivilegedMode() {
    u8 current_mode = regs.cpsr & 0x1F;
    return (current_mode != USR);
}

bool CPUCore::IsARM() {
    return (!(regs.cpsr & (1 << 5)));
}

void CPUCore::SwitchMode(u8 new_mode) {
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

void CPUCore::HandleInterrupt() {
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
}

void CPUCore::arm_coprocessor_register_transfer() {
    if (arch == CPUArch::ARMv4) {
        return;
    }

    u8 crm = instruction & 0xF;
    u8 crn = (instruction >> 16) & 0xF;
    u8 opcode2 = (instruction >> 5) & 0x7;
    u8 rd = (instruction >> 12) & 0xF;

    if (instruction & (1 << 20)) {
        regs.r[rd] = cp15->Read(crn, crm, opcode2);

        if (rd == 15) {
            log_fatal("handle");
        }
    } else {
        cp15->Write(crn, crm, opcode2, regs.r[rd]);
    }

    regs.r[15] += 4;
}

void CPUCore::arm_software_interrupt() {
    regs.spsr_banked[BANK_SVC] = regs.cpsr;

    SwitchMode(SVC);

    regs.cpsr |= (1 << 7);
    regs.r[14] = regs.r[15] - 4;

    // jump to the exception base in the bios
    regs.r[15] = ((arch == CPUArch::ARMv5) ? cp15->GetExceptionBase() : 0x00000000) + 0x08;
    ARMFlushPipeline();
}

void CPUCore::thumb_software_interrupt() {
    regs.spsr_banked[BANK_SVC] = regs.cpsr;

    SwitchMode(SVC);

    regs.cpsr &= ~(1 << 5);
    regs.cpsr |= (1 << 7);
    regs.r[14] = regs.r[15] - 2;

    // jump to the exception base in the bios
    regs.r[15] = ((arch == CPUArch::ARMv5) ? cp15->GetExceptionBase() : 0x00000000) + 0x08;
    ARMFlushPipeline();
}

void CPUCore::ARMUndefinedException() {
    regs.spsr_banked[BANK_UND] = regs.cpsr;

    SwitchMode(UND);

    regs.cpsr |= (1 << 7);
    regs.r[14] = regs.r[15] - 4;

    // jump to the exception base in the bios
    regs.r[15] = ((arch == CPUArch::ARMv5) ? cp15->GetExceptionBase() : 0x00000000) + 0x04;
    ARMFlushPipeline();
}

void CPUCore::unknown_instruction() {
    log_fatal("[Interpreter] Instruction (%08x) is unimplemented at r15 = %08x", instruction, regs.r[15]);
}

void CPUCore::log_cpu_state() {
    for (int i = 0; i < 16; i++) {
        LogFile::Get().Log("%08x ", regs.r[i]);
    }

    LogFile::Get().Log("%08x\n", instruction);
}