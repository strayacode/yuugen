#pragma once

#include <core/arm/cpu_base.h>
#include <core/arm/cpu_registers.h>
#include <core/arm/memory_base.h>
#include <common/types.h>
#include <common/log.h>
#include <array>

enum class CPUArch {
    ARMv4 = 0,
    ARMv5 = 1,
};

enum Mode {
    USR = 0x10,
    FIQ = 0x11,
    IRQ = 0x12,
    SVC = 0x13,
    ABT = 0x17,
    UND = 0x1B,
    SYS = 0x1F, // "privileged" user mode
};

enum Bank {
    BANK_FIQ = 0,
    BANK_IRQ = 1,
    BANK_SVC = 2,
    BANK_ABT = 3,
    BANK_UND = 4,
};

enum ConditionFlag {
    N_FLAG = 31,
    Z_FLAG = 30,
    C_FLAG = 29,
    V_FLAG = 28,
    Q_FLAG = 27,
};

enum CPUCondition {
    CONDITION_EQ = 0,
    CONDITION_NE = 1,
    CONDITION_CS = 2,
    CONDITION_CC = 3,
    CONDITION_MI = 4,
    CONDITION_PL = 5,
    CONDITION_VS = 6,
    CONDITION_VC = 7,
    CONDITION_HI = 8,
    CONDITION_LS = 9,
    CONDITION_GE = 10,
    CONDITION_LT = 11,
    CONDITION_GT = 12,
    CONDITION_LE = 13,
    CONDITION_AL = 14,
    CONDITION_NV = 15,
};

class Interpreter : public CPUBase {
public:
    Interpreter(MemoryBase& memory, CPUArch arch);
    ~Interpreter() override;

    void Reset() override;
    void Run(int cycles) override;

    typedef void (Interpreter::*Instruction)();
private:
    void Execute();

    void ARMFlushPipeline();
    void ThumbFlushPipeline();

    auto GetCurrentSPSR() -> u32;
    bool HasSPSR();
    bool PrivilegedMode();

    auto Halted() -> bool;

    bool IsARM();

    auto ReadByte(u32 addr) -> u8;
    auto ReadHalf(u32 addr) -> u16;
    auto ReadWord(u32 addr) -> u32;

    void WriteByte(u32 addr, u8 data);
    void WriteHalf(u32 addr, u16 data);
    void WriteWord(u32 addr, u32 data);

    CPURegisters regs;

    u32 pipeline[2];
    u32 instruction;

    bool halted;

    static std::array<Instruction, 1024> thumb_lut;
    static std::array<Instruction, 4096> arm_lut;

    MemoryBase& memory;

    CPUArch arch;
};