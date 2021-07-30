#include <core/arm/interpreter/interpreter.h>

Interpreter::Interpreter(MemoryBase& memory, CPUArch arch) : memory(memory), arch(arch) {

}

Interpreter::~Interpreter() {

}

void Interpreter::Reset() {

}

void Interpreter::Run(int cycles) {
    while (cycles--) {
        // stepping the pipeline must happen before an instruction is executed incase the instruction is a branch which would flush and then step the pipeline (not correct)
        instruction = pipeline[0]; // store the current executing instruction 
        // shift the pipeline
        pipeline[0] = pipeline[1];
        // fill the 2nd item with the new instruction to be read
        if (IsARM()) {
            pipeline[1] = ReadWord(regs.r[15]);
        } else {
            pipeline[1] = ReadHalf(regs.r[15]);
        }

        // TODO: replace with lookup table
        // Execute();
    }
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

auto Interpreter::Halted() -> bool {
    return halted;
}

bool Interpreter::IsARM() {
    return (!(regs.cpsr & (1 << 5)));
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