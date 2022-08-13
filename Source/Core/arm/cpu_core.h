#pragma once

#include <array>
#include "Common/arithmetic.h"
#include "Common/log_file.h"
#include "Core/arm/cpu_registers.h"
#include "Core/arm/memory_base.h"
#include "Core/arm/Decoder/Decoder.h"
#include "Core/arm/ARMTypes.h"

enum class InterruptType {
    VBlank = 0,
    HBlank = 1,
    VCounter = 2,
    Timer0 = 3,
    Timer1 = 4,
    Timer2 = 5,
    Timer3 = 6,
    RTC = 7,
    DMA0 = 8,
    DMA1 = 9,
    DMA2 = 10,
    DMA3 = 11,
    Input = 12,
    IPCSync = 16,
    IPCSendEmpty = 17,
    IPCReceiveNonEmpty = 18,
    CartridgeTransfer = 19,
    GXFIFO = 21,
    SPI = 23,  
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
    BANK_USR = 0,
    BANK_FIQ = 1,
    BANK_IRQ = 2,
    BANK_SVC = 3,
    BANK_ABT = 4,
    BANK_UND = 5,
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

class CPUCore;

typedef void (CPUCore::*Instruction)();

class CP15;

// this is core class which is used by
// both the arm7 and arm9 and can be used
// for cores such as an interpreter,
// cached interpreter and jit
class CPUCore {
public:
    CPUCore(MemoryBase& memory, Arch arch, CP15* cp15);
    
    void Reset();

    void run(u64 target);
    int single_step();

    void DirectBoot(u32 entrypoint);
    void FirmwareBoot();
    void SendInterrupt(InterruptType interrupt_type);
    void Halt();
    bool Halted();
    void ARMFlushPipeline();
    void ThumbFlushPipeline();
    void GenerateConditionTable();
    bool ConditionEvaluate(u8 condition);
    bool GetConditionFlag(int condition_flag);
    void SetConditionFlag(int condition_flag, int value);
    u32 GetCurrentSPSR();
    void SetCurrentSPSR(u32 data);
    bool HasSPSR();
    bool PrivilegedMode();
    bool IsARM();
    void SwitchMode(u8 new_mode);
    void HandleInterrupt();
    void arm_software_interrupt();
    void thumb_software_interrupt();
    void ARMUndefinedException();
    void arm_coprocessor_register_transfer();
    void unknown_instruction();

    #include "interpreter/instructions/arm/alu.inl"
    #include "interpreter/instructions/arm/branch.inl"
    #include "interpreter/instructions/arm/load_store.inl"
    #include "interpreter/instructions/thumb/alu.inl"
    #include "interpreter/instructions/thumb/branch.inl"
    #include "interpreter/instructions/thumb/load_store.inl"

    u8 ReadByte(u32 addr);
    u16 ReadHalf(u32 addr);
    u32 ReadWord(u32 addr);
    u32 ReadWordRotate(u32 addr);

    void WriteByte(u32 addr, u8 data);
    void WriteHalf(u32 addr, u16 data);
    void WriteWord(u32 addr, u32 data);

    void log_cpu_state();

    CPURegisters regs;

    bool halted = true;

    u32 irf;
    u32 ie;
    u32 ime;

    MemoryBase& memory;
    Arch arch;
    u32 instruction;

    // condition table for every possible condition of the bits 28..31 in an opcode
    // so for each type of condition code we have 2^4 possibilities
    std::array<std::array<bool, 16>, 16> condition_table;

    u32 pipeline[2];

    CP15* cp15;

private:
    int instruction_cycles = 0;
    u64 timestamp = 0;

    void add_internal_cycles(int cycles);
};