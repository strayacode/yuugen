#pragma once

#include <memory>
#include "Common/Types.h"
#include "Core/ARM/CPUBase.h"
#include "Core/ARM/Interpreter/Interpreter.h"
#include "Core/ARM/ARM7/Memory.h"
#include "Core/ARM/ARM7/Coprocessor.h"

class System;

class ARM7 {
public:
    ARM7(System& system);

    void select_backend(CPUBackend backend);

    inline ARM7Memory& memory() { return m_memory; }
    inline ARM7Coprocessor& coprocessor() { return m_coprocessor; }
    inline CPUBase& cpu() { return *m_cpu; }
    
private:
    ARM7Memory m_memory;
    ARM7Coprocessor m_coprocessor;
    std::unique_ptr<CPUBase> m_cpu;
};
