#include "Core/ARM/ARM7/ARM7.h"
#include "Core/System.h"

ARM7::ARM7(System& system) : m_memory(system), m_cpu(m_memory, m_coprocessor, Arch::ARMv4) {}
