#include "Core/ARM/Interpreter/Interpreter.h"

Interpreter::Interpreter(CPU& cpu) : m_cpu(cpu) {}

u64 Interpreter::run(u64 target) {

}

template <typename T>
T Interpreter::read(u32 addr) {
    return m_cpu.m_memory.read<T>(addr);
}

template <typename T>
void Interpreter::write(u32 addr, T data) {
    m_cpu.m_memory.write<T>(addr, data);
}
