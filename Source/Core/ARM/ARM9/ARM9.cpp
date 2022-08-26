#include "Core/ARM/ARM9/ARM9.h"
#include "Core/System.h"

ARM9::ARM9(System& system) : m_memory(system), m_coprocessor(system), m_cpu(m_memory, m_coprocessor, Arch::ARMv5) {}