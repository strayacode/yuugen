#pragma once

#include <core/arm/cpu_registers.h>
#include <core/arm/memory_base.h>

enum class CPUArch {
    ARMv4 = 0,
    ARMv5 = 1,
};

// this is core class which is used by
// both the arm7 and arm9 and can be used
// for cores such as an interpreter,
// cached interpreter and jit
class CPUCore {
public:
    CPUCore(MemoryBase& memory, CPUArch arch);
    
    void Reset();
    void RunInterpreter(int cycles);
    void DirectBoot(u32 entrypoint);
    void FirmwareBoot();
    void SendInterrupt(int interrupt);
    void Halt();
    bool Halted();

    u8 ReadByte(u32 addr);
    u16 ReadHalf(u32 addr);
    u32 ReadWord(u32 addr);
    u32 ReadWordRotate(u32 addr);

    void WriteByte(u32 addr, u8 data);
    void WriteHalf(u32 addr, u16 data);
    void WriteWord(u32 addr, u32 data);

    CPURegisters regs;

    bool halted = true;

    u32 irf;
    u32 ie;
    u32 ime;

    MemoryBase& memory;

    CPUArch arch;
};