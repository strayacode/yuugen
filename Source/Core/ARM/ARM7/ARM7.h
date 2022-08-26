#pragma once

#include "Common/Types.h"
#include "Core/ARM/CPU.h"
#include "Core/ARM/ARM7/Memory.h"
#include "Core/ARM/ARM7/Coprocessor.h"

class System;

class ARM7 {
public:
    ARM7(System& system);

    inline ARM7Memory& memory() { return m_memory; }
    inline ARM7Coprocessor& coprocessor() { return m_coprocessor; }
    inline CPU& cpu() { return m_cpu; }
    
private:
    ARM7Memory m_memory;
    ARM7Coprocessor m_coprocessor;
    CPU m_cpu;
};
