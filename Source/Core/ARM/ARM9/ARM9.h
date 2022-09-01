#pragma once

#include <memory>
#include "Common/Types.h"
#include "Core/ARM/CPUBase.h"
#include "Core/ARM/Interpreter/Interpreter.h"
#include "Core/ARM/ARM9/Memory.h"
#include "Core/ARM/ARM9/Coprocessor.h"

class System;

class ARM9 {
public:
    ARM9(System& system);

    void select_backend(CPUBackend backend);

    inline ARM9Memory& memory() { return m_memory; }
    inline ARM9Coprocessor& coprocessor() { return m_coprocessor; }
    inline CPUBase& cpu() { return *m_cpu; }

private:
    ARM9Memory m_memory;
    ARM9Coprocessor m_coprocessor;
    std::unique_ptr<CPUBase> m_cpu;
};
