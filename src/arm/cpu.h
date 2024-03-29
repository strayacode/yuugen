#pragma once

#include "common/types.h"
#include "arm/state.h"
#include "arm/arch.h"

namespace arm {

enum GPR : int {
    R0 = 0,
    R1 = 1,
    R2 = 2,
    R3 = 3,
    R4 = 4,
    R5 = 5,
    R6 = 6,
    R7 = 7,
    R8 = 8,
    R9 = 9,
    R10 = 10,
    R11 = 11,
    R12 = 12,
    R13 = 13,
    R14 = 14,
    R15 = 15,
    SP = R13,
    LR = R14,
    PC = R15,
};

enum Bank : u8 {
    USR = 0,
    FIQ = 1,
    IRQ = 2,
    SVC = 3,
    ABT = 4,
    UND = 5,
};

enum Condition : u8 {
    EQ = 0,
    NE = 1,
    CS = 2,
    CC = 3,
    MI = 4,
    PL = 5,
    VS = 6,
    VC = 7,
    HI = 8,
    LS = 9,
    GE = 10,
    LT = 11,
    GT = 12,
    LE = 13,
    AL = 14,
    NV = 15,
};

enum class BackendType {
    Interpreter,
    IRInterpreter,
    Jit,
};

class CPU {
public:
    virtual ~CPU() = default;
    virtual void reset() = 0;
    virtual void run(int cycles) = 0;
    virtual void update_irq(bool irq) = 0;
    virtual bool is_halted() = 0;
    virtual void update_halted(bool halted) = 0;
    virtual Arch get_arch() = 0;

    virtual u32 get_gpr(GPR gpr) = 0;
    virtual u32 get_gpr(GPR gpr, Mode mode) = 0;
    virtual void set_gpr(GPR gpr, u32 value) = 0;
    virtual void set_gpr(GPR gpr, Mode mode, u32 value) = 0;

    virtual StatusRegister get_cpsr() = 0;
    virtual void set_cpsr(StatusRegister value) = 0;
    virtual StatusRegister get_spsr(Mode mode) = 0;
    virtual void set_spsr(Mode mode, StatusRegister value) = 0;

    State state;
};

} // namespace arm