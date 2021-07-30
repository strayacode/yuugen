#pragma once

#include <core/arm/cpu_registers.h>
#include <core/arm/memory_base.h>

enum class CPUArch {
    ARMv4 = 0,
    ARMv5 = 1,
};

// this is a base class which is used by
// both the arm7 and arm9 and can be used
// for cores such as an interpreter,
// cached interpreter and jit
class CPUBase {
public:
    CPUBase(MemoryBase& memory, CPUArch arch);
    virtual ~CPUBase() {};
    virtual void Reset() = 0;
    virtual void Run(int cycles) = 0;
    virtual void DirectBoot(u32 entrypoint) = 0;

    void SendInterrupt(int interrupt);
    void Halt();
    auto Halted() -> bool;

    CPURegisters regs;

    bool halted;

    u32 irf;
    u32 ie;
    u32 ime;

    MemoryBase& memory;

    CPUArch arch;
};