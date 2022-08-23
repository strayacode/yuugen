#pragma once

#include <memory>
#include "Common/Types.h"
#include "Core/ARM/ExecutorInterface.h"
#include "Core/ARM/MemoryBase.h"
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
    
private:
    std::unique_ptr<ExecutorInterface> executor;

    MemoryBase memory;
    CoprocessorBase coprocessor;
};
