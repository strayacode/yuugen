#pragma once

#include <common/types.h>
#include <common/log.h>
#include <algorithm>

// probably change these...
#define ADD_CARRY(a, b)  ((0xFFFFFFFF-a) < b)
#define SUB_CARRY(a, b)  (a >= b)

#define ADD_OVERFLOW(a, b, res)  ((!(((a) ^ (b)) & 0x80000000)) && (((a) ^ (res)) & 0x80000000))
#define SUB_OVERFLOW(a, b, res)  ((((a) ^ (b)) & 0x80000000) && (((a) ^ (res)) & 0x80000000))

#define INSTRUCTION(NAME, ...) void NAME(__VA_ARGS__)

class HW;

class ARM7 {
public:
    ARM7(HW* hw, int arch);

    enum CPUArch {
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

    void Reset();

    void DirectBoot();
    void FirmwareBoot();

    auto ReadByte(u32 addr) -> u8;
    auto ReadHalf(u32 addr) -> u16;
    auto ReadWord(u32 addr) -> u32;

    void WriteByte(u32 addr, u8 data);
    void WriteHalf(u32 addr, u16 data);
    void WriteWord(u32 addr, u32 data);

    bool HasSPSR();
    bool PrivilegedMode();

    void SwitchMode(u8 new_mode);

    void ARMFlushPipeline();
    void ThumbFlushPipeline();

    void Step();
    void Execute();

    auto Halted() -> bool;

    bool IsARM();

    void GenerateConditionTable();
    bool ConditionEvaluate(u8 condition);

    bool GetConditionFlag(int condition_flag);
    void SetConditionFlag(int condition_flag, int value);

    void Halt();

    void SendInterrupt(int interrupt);
    void HandleInterrupt();

    auto GetCurrentSPSR() -> u32;
    void SetCurrentSPSR(u32 data);

    // instructions that require access to core we just declare outside the .inl files
    void ARM_MRC();
    void ARM_MCR();
    void ARM_SWI();
    void ARM_UND();

    void THUMB_SWI();

    void LogRegisters();
    void DebugRegisters();

    #include "instructions/arm/block_data_transfer.inl"
    #include "instructions/arm/branch.inl"
    #include "instructions/arm/data_processing.inl"
    #include "instructions/arm/multiply.inl"
    #include "instructions/arm/software_interrupt.inl"
    #include "instructions/arm/misc.inl"
    #include "instructions/arm/single_data_transfer.inl"
    #include "instructions/arm/halfword_signed_transfer.inl"
    #include "instructions/arm/psr_transfer.inl"

    #include "instructions/thumb/block_data_transfer.inl"
    #include "instructions/thumb/branch.inl"
    #include "instructions/thumb/data_processing.inl"
    #include "instructions/thumb/halfword_signed_transfer.inl"
    #include "instructions/thumb/single_data_transfer.inl"

    struct CPURegisters {
        // 16 general purpose registers
        u32 r[16];

        // this stores 6 banks of registers from r8-r14
        u32 r_banked[6][7];

        // the current program status register
        u32 cpsr;

        // the current spsr
        u32 spsr;

        // this is for the banked spsrs
        u32 spsr_fiq;
        u32 spsr_svc;
        u32 spsr_abt;
        u32 spsr_irq;
        u32 spsr_und;

    } regs;

    u32 pipeline[2];

    u32 instruction;

    HW* hw;

    int arch;

    bool halted = false;

    // condition table for every possible condition of the bits 28..31 in an opcode
    // so for each type of condition code we have 2^4 possibilities
    bool condition_table[16][16] = {};

    FILE* log_buffer;
    int counter = 0;
};