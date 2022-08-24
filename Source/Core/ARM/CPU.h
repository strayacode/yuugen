#pragma once

#include <memory>
#include "Common/Types.h"
#include "Core/ARM/ExecutorInterface.h"
#include "Core/ARM/MemoryBase.h"
#include "Core/ARM/CoprocessorBase.h"
#include "Core/ARM/ARMTypes.h"

enum class ExecutorType {
    Interpreter,
};

class System;

class CPU {
public:
    CPU(System& system, Arch arch);

    void reset();
    void direct_boot();
    void firmware_boot();
    void run(u64 target); 
    void select_executor(ExecutorType executor_type);
    
    inline MemoryBase& memory() { return *m_memory; }
    inline CoprocessorBase& coprocessor() { return *m_coprocessor; }
    
private:
    std::unique_ptr<ExecutorInterface> m_executor;
    std::unique_ptr<MemoryBase> m_memory;
    std::unique_ptr<CoprocessorBase> m_coprocessor;
};
