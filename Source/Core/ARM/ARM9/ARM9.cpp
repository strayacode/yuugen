#include "Core/ARM/ARM9/ARM9.h"
#include "Core/System.h"

ARM9::ARM9(System& system) : m_memory(system), m_coprocessor(system) {}

void ARM9::select_backend(CPUBackend backend) {
    switch (backend) {
    case CPUBackend::Interpreter:
        m_cpu = std::make_unique<Interpreter>(m_memory, m_coprocessor, Arch::ARMv5);
        break;
    default:
        log_fatal("[ARM9] handle unknown backend");
    }
}