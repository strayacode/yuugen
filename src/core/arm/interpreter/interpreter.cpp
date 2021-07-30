#include <core/arm/interpreter/interpreter.h>

Interpreter::Interpreter(MemoryBase& memory, CPUArch arch) : memory(memory), arch(arch) {

}

Interpreter::~Interpreter() {

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