#include "core/arm/cpu_core.h"

CPUCore::CPUCore(MemoryBase& memory, CPUArch arch) : memory(memory), arch(arch) {}

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
}

void CPUCore::RunInterpreter(int cycles) {
    while (cycles-- > 0) {
        if (halted) {
            return;
        }

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

        if (ime && (ie & irf) && !(regs.cpsr & (1 << 7))) {
            HandleInterrupt();
        }

        if (IsARM()) {
            u32 index = ((instruction >> 16) & 0xFF0) | ((instruction >> 4) & 0xF);
            if (ConditionEvaluate(instruction >> 28)) {
                (this->*arm_lut[index])();
            } else {
                regs.r[15] += 4;
            }
        } else {
            u32 index = instruction >> 6;
            (this->*thumb_lut[index])();
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
    
void CPUCore::SendInterrupt(int interrupt) {
    // set the appropriate bit in IF
    irf |= (1 << interrupt);
    
    // check if the interrupt is enabled too
    if (ie & (1 << interrupt)) {
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