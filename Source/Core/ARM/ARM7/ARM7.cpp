#include "Core/ARM/ARM7/ARM7.h"
#include "Core/System.h"

ARM7::ARM7(System& system) : m_memory(system) {}

void ARM7::select_backend(CPUBackend backend) {
    switch (backend) {
    case CPUBackend::Interpreter:
        m_cpu = std::make_unique<Interpreter>(m_memory, m_coprocessor, Arch::ARMv4);
        break;
    default:
        log_fatal("[ARM7] handle unknown backend");
    }
}