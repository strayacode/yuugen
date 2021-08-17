#pragma once

#include <core/arm/cpu_base.h>
#include <common/types.h>
#include <common/log.h>
#include <common/arithmetic.h>
#include <common/log_file.h>
#include <array>
#include <memory>

// probably change these...
#define ADD_CARRY(a, b)  ((0xFFFFFFFF-a) < b)
#define SUB_CARRY(a, b)  (a >= b)

#define ADD_OVERFLOW(a, b, res)  ((!(((a) ^ (b)) & 0x80000000)) && (((a) ^ (res)) & 0x80000000))
#define SUB_OVERFLOW(a, b, res)  ((((a) ^ (b)) & 0x80000000) && (((a) ^ (res)) & 0x80000000))

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

class Interpreter;

typedef void (Interpreter::*Instruction)();

class CP15;

class Interpreter : public CPUBase {
public:
    Interpreter(MemoryBase& memory, CPUArch arch, CP15* cp15);
    ~Interpreter() override;

    void Reset() override;
    void Run(int cycles) override;
    void DirectBoot(u32 entrypoint) override;
    void FirmwareBoot() override;

    #include "instructions/arm/alu.inl"
    #include "instructions/arm/branch.inl"
    #include "instructions/arm/load_store.inl"

    #include "instructions/thumb/alu.inl"
    #include "instructions/thumb/branch.inl"
    #include "instructions/thumb/load_store.inl"
private:
    void ARMFlushPipeline();
    void ThumbFlushPipeline();

    auto GetCurrentSPSR() -> u32;
    void SetCurrentSPSR(u32 data);
    bool HasSPSR();
    bool PrivilegedMode();

    bool IsARM();

    void GenerateARMTable();
    void GenerateThumbTable();

    void UnimplementedInstruction();

    void GenerateConditionTable();
    bool ConditionEvaluate(u8 condition);

    void SwitchMode(u8 new_mode);

    bool GetConditionFlag(int condition_flag);
    void SetConditionFlag(int condition_flag, int value);

    void LogRegisters();

    void HandleInterrupt();
    void ARMSoftwareInterrupt();
    void ARM_UND();

    void THUMB_SWI();

    void ARMCoprocessorRegisterTransfer();

    auto ReadByte(u32 addr) -> u8;
    auto ReadHalf(u32 addr) -> u16;
    auto ReadWord(u32 addr) -> u32;

    void WriteByte(u32 addr, u8 data);
    void WriteHalf(u32 addr, u16 data);
    void WriteWord(u32 addr, u32 data);

    u32 pipeline[2];
    u32 instruction;

    // condition table for every possible condition of the bits 28..31 in an opcode
    // so for each type of condition code we have 2^4 possibilities
    std::array<std::array<bool, 16>, 16> condition_table;

    // TODO: handle differences in sending interrupt for arm7 and arm9

    std::unique_ptr<LogFile> log_file;

    CP15* cp15;

    std::array<Instruction, 1024> thumb_lut;
    std::array<Instruction, 4096> arm_lut;
};