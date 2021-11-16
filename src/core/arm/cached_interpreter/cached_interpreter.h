#pragma once

#include <core/arm/cpu_base.h>
#include <common/types.h>
#include <common/log.h>
#include <common/arithmetic.h>
#include <common/log_file.h>
#include <array>
#include <memory>

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

class CP15;

class CachedInterpreter : public CPUBase {
public:
    CachedInterpreter(MemoryBase& memory, CPUArch arch, CP15* cp15);
    ~CachedInterpreter() override;

    void Reset() override;
    void Run(int cycles) override;
    void DirectBoot(u32 entrypoint) override;
    void FirmwareBoot() override;

    CP15* cp15;
};