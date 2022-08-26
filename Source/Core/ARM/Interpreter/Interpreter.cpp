#include "Core/ARM/Interpreter/Interpreter.h"

Interpreter::Interpreter(MemoryBase& memory, CoprocessorBase& coprocessor, Arch arch) : CPUBase(memory, coprocessor, arch) {}

void Interpreter::run(u64 target) {
    log_fatal("handle interpreter run");

    while (m_timestamp < target) {
        if (m_halted) {
            m_timestamp = target;
            return;
        }

        // m_timestamp += m_executor->run(target);
    }
}

template <typename T>
T Interpreter::read(u32 addr) {
    return m_memory.read<T>(addr);
}

template <typename T>
void Interpreter::write(u32 addr, T data) {
    m_memory.write<T>(addr, data);
}

void Interpreter::arm_flush_pipeline() {
    m_gpr[15] &= ~3;
    m_pipeline[0] = read<u32>(m_gpr[15]);
    m_pipeline[1] = read<u32>(m_gpr[15] + 4);
    m_gpr[15] += 8;
}

void Interpreter::thumb_flush_pipeline() {
    m_gpr[15] &= ~1;
    m_pipeline[0] = read<u16>(m_gpr[15]);
    m_pipeline[1] = read<u16>(m_gpr[15] + 2);
    m_gpr[15] += 4;
}