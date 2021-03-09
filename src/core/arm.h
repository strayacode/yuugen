#pragma once

#include <stdio.h>
#include <common/types.h>
#include <common/log.h>
#include <string.h>
#include <stdlib.h>


struct Core;

#define ADD_CARRY(a, b)  ((0xFFFFFFFF-a) < b)
#define SUB_CARRY(a, b)  (a >= b)

#define ADD_OVERFLOW(a, b, res)  ((!(((a) ^ (b)) & 0x80000000)) && (((a) ^ (res)) & 0x80000000))
#define SUB_OVERFLOW(a, b, res)  ((((a) ^ (b)) & 0x80000000) && (((a) ^ (res)) & 0x80000000))

#define INSTRUCTION(NAME, ...) void NAME(__VA_ARGS__)

// // a carry will occur from an addition if the result is less than either of the operands
// #define ADD_CARRY(op1, op2, result) (op1 > result)

// // no borrow will occur in a subtraction if the first operand is larger than the second
// #define SUB_CARRY(op1, op2) (op1 >= op2)


// // a signed overflow from addition will occur if the 2 operands have the same sign and the result has a different sign
// #define ADD_OVERFLOW(op1, op2, result) (((!(op1 ^ op2)) >> 31) && ((op1 ^ result) >> 31))
// #define SUB_OVERFLOW(op1, op2, result) (((op1 ^ op2) >> 31) && ((op1 ^ result) >> 31))

enum CPUMode {
    USR = 0x10,
    FIQ = 0x11,
    IRQ = 0x12,
    SVC = 0x13,
    ABT = 0x17,
    UND = 0x1B,
    SYS = 0x1F, // "privileged" user mode
};

// non-class enum as we require them for indexes in an array
enum RegisterBank {
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

enum CPUArchitecture {
    ARMv4 = 0,
    ARMv5 = 1,
};

struct ARM {
    ARM(Core* core, int arch);

    void Reset();
    void DirectBoot();

    // 0 = arm7, 1 = arm9
    int arch;

    struct CPURegisters {
        // 16 general purpose registers
        u32 r[16];

        // this stores 6 banks of registers from r8-r14
        u32 r_banked[6][7];

        // the current program status register
        u32 cpsr;

        // the current spsr
        u32 spsr;

        // this stores 6 banks of spsr
        u32 spsr_banked[6];
    } regs;

    Core* core;

    // this is used for emulating the pipeline. we store the raw instruction data of the instruction currently executing and the instruction currently being decoded
    u32 pipeline[2];

    u32 instruction = 0;

    bool IsARM();
    
    bool GetConditionFlag(int condition_flag);
    void SetConditionFlag(int condition_flag, int value);

    bool ConditionEvaluate();

    void ARMFlushPipeline();
    void ThumbFlushPipeline();
    void ExecuteInstruction();
    void Step();

    u8 ReadByte(u32 addr);
    u16 ReadHalfword(u32 addr);
    u32 ReadWord(u32 addr);
    void WriteByte(u32 addr, u8 data);
    void WriteHalfword(u32 addr, u16 data);
    void WriteWord(u32 addr, u32 data);

    void DebugRegisters();
    void LogRegisters();

    int GetRegisterBank(int mode);
    void UpdateMode(int new_mode);
    
    #include <core/arm_alu.inl>
    #include <core/arm_branch.inl>
    #include <core/arm_transfer.inl>
    #include <core/thumb_alu.inl>
    #include <core/thumb_branch.inl>
    #include <core/thumb_transfer.inl>

    void Halt();
    void SendInterrupt(int interrupt);
    void HandleInterrupt();

    // instructions that require access to core we just declare outside the .inl files
    void ARM_MRC();
    void ARM_MCR();
    void ARM_SWI();

    void THUMB_SWI();

    FILE* log_buffer;
    int counter = 0;

    bool halted;

};




