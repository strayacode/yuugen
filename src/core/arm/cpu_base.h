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
    CPUBase(MemoryBase& memory, CPUArch arch) : memory(memory), arch(arch) {}
    virtual ~CPUBase() {};
    virtual void Reset() = 0;
    virtual void Run(int cycles) = 0;
    virtual void DirectBoot(u32 entrypoint) = 0;
    virtual void FirmwareBoot() = 0;

    void SendInterrupt(int interrupt) {
        // set the appropriate bit in IF
        irf |= (1 << interrupt);
        
        // check if the interrupt is enabled too
        if (ie & (1 << interrupt)) {
            // to unhalt on the arm9 ime needs to be set
            if (ime || arch == CPUArch::ARMv4) {
                halted = false;
            }
        }
    }

    void Halt() {
        halted = true;
    }

    bool Halted() {
        return halted;
    }

    CPURegisters regs;

    bool halted = true;

    u32 irf;
    u32 ie;
    u32 ime;

    MemoryBase& memory;

    CPUArch arch;
};