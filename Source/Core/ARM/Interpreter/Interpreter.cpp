#include "Core/ARM/Interpreter/Interpreter.h"

Interpreter::Interpreter(CPU& cpu) : m_cpu(cpu) {}

void Interpreter::run(u64 target) {

}

template <typename T>
T Interpreter::read(u32 addr) {
    return m_cpu.memory().read<T>(addr);
}

template <typename T>
void Interpreter::write(u32 addr, T data) {
    m_cpu.memory().write<T>(addr, data);
}
