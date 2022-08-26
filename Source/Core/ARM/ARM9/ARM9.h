#pragma once

#include "Common/Types.h"
#include "Core/ARM/CPU.h"
#include "Core/ARM/ARM9/Memory.h"
#include "Core/ARM/ARM9/Coprocessor.h"

class System;

class ARM9 {
public:
    ARM9(System& system);

    inline ARM9Memory& memory() { return m_memory; }
    inline ARM9Coprocessor& coprocessor() { return m_coprocessor; }
    inline CPU& cpu() { return m_cpu; }

private:
    ARM9Memory m_memory;
    ARM9Coprocessor m_coprocessor;
    CPU m_cpu;
};
