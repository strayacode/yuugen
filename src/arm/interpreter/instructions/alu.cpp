#include "common/bits.h"
#include "arm/instructions.h"
#include "arm/interpreter/interpreter.h"

namespace arm {

u32 Interpreter::barrel_shifter(u32 value, ShiftType shift_type, int amount, bool& carry, bool imm) {
    switch (shift_type) {
    case ShiftType::LSL:
        return alu_lsl(value, amount, carry);
    case ShiftType::LSR:
        return alu_lsr(value, amount, carry, imm);
    case ShiftType::ASR:
        return alu_asr(value, amount, carry, imm);
    case ShiftType::ROR:
        return alu_ror(value, amount, carry, imm);
    }
    
    return value;
}

u32 Interpreter::alu_mov(u32 op2, bool set_flags) {
    if (set_flags) {
        set_nz(op2);
    }

    return op2;
}

u32 Interpreter::alu_mvn(u32 op2, bool set_flags) {
    u32 result = ~op2;
    if (set_flags) {
        set_nz(result);
    }

    return result;
}

void Interpreter::alu_teq(u32 op1, u32 op2) {
    set_nz(op1 ^ op2);
}

void Interpreter::alu_cmp(u32 op1, u32 op2) {
    u32 result = op1 - op2;
    set_nz(result);
    state.cpsr.c = op1 >= op2;
    state.cpsr.v = calculate_sub_overflow(op1, op2, result);
}

void Interpreter::alu_cmn(u32 op1, u32 op2) {
    u32 result = op1 + op2;
    set_nz(result);
    state.cpsr.c = result < op1;
    state.cpsr.v = calculate_add_overflow(op1, op2, result);
}

void Interpreter::alu_tst(u32 op1, u32 op2) {
    set_nz(op1 & op2);
}

u32 Interpreter::alu_add(u32 op1, u32 op2, bool set_flags) {
    u32 result = op1 + op2;
    if (set_flags) {
        set_nz(result);
        state.cpsr.c = result < op1;
        state.cpsr.v = calculate_add_overflow(op1, op2, result);
    }

    return result;
}

u32 Interpreter::alu_adc(u32 op1, u32 op2, bool set_flags) {
    u64 result64 = (u64)op1 + (u64)op2 + (u64)state.cpsr.c;
    u32 result = (u32)result64;
    if (set_flags) {
        set_nz(result);
        state.cpsr.c = result64 >> 32;
        state.cpsr.v = calculate_add_overflow(op1, op2, result);
    }

    return result;
}

u32 Interpreter::alu_sbc(u32 op1, u32 op2, bool set_flags) {
    u32 op3 = state.cpsr.c ^ 1;
    u32 result = op1 - op2 - op3;
    if (set_flags) {
        set_nz(result);
        state.cpsr.c = (u64)op1 >= (u64)op2 + (u64)op3;
        state.cpsr.v = calculate_sub_overflow(op1, op2, result);
    }

    return result;
}

u32 Interpreter::alu_eor(u32 op1, u32 op2, bool set_flags) {
    u32 result = op1 ^ op2;
    if (set_flags) {
        set_nz(result);
    }

    return result;
}

u32 Interpreter::alu_sub(u32 op1, u32 op2, bool set_flags) {
    u32 result = op1 - op2;
    if (set_flags) {
        set_nz(result);
        state.cpsr.c = op1 >= op2;
        state.cpsr.v = calculate_sub_overflow(op1, op2, result);
    }

    return result;
}

u32 Interpreter::alu_orr(u32 op1, u32 op2, bool set_flags) {
    u32 result = op1 | op2;
    if (set_flags) {
        set_nz(result);
    }

    return result;
}

u32 Interpreter::alu_bic(u32 op1, u32 op2, bool set_flags) {
    u32 result = op1 & ~op2;
    if (set_flags) {
        set_nz(result);
    }

    return result;
}

u32 Interpreter::alu_and(u32 op1, u32 op2, bool set_flags) {
    u32 result = op1 & op2;
    if (set_flags) {
        set_nz(result);
    }

    return result;
}

u32 Interpreter::alu_lsl(u32 value, int amount, bool& carry) {
    if (amount == 0) {
        return value;
    }

    u32 result = 0;
    if (amount >= 32) {
        if (amount > 32) {
            carry = 0;
        } else {
            carry = value & 0x1;
        }
    } else {
        result = value << amount;
        carry = (value >> (32 - amount)) & 0x1;
    }

    return result;
}

u32 Interpreter::alu_lsr(u32 value, int amount, bool& carry, bool imm) {
    u32 result = 0;

    if (imm) {
        if (amount == 0) {
            result = 0;
            carry = value >> 31;
        } else {
            result = value >> amount;
            carry = (value >> (amount - 1)) & 0x1;
        }
    } else {
        if (amount == 0) {
            result = value;
        } else if (amount < 32) {
            result = value >> amount;
            carry = (value >> (amount - 1)) & 0x1;
        } else if (amount == 32) {
            result = 0;
            carry = value >> 31;
        } else {
            result = 0;
            carry = false;
        }
    }

    return result;
}

u32 Interpreter::alu_asr(u32 value, int amount, bool& carry, bool imm) {
    u32 result = 0;
    u8 msb = value >> 31;

    if (imm) {
        if (amount == 0) {
            result = 0xffffffff * msb;
            carry = msb;
        } else {
            result = (value >> amount) | ((0xffffffff * msb) << (32 - amount));
            carry = (value >> (amount - 1)) & 0x1;
        }
    } else {
        if (amount == 0) {
            result = value;
        } else if (amount < 32) {
            result = (value >> amount) | ((0xffffffff * msb) << (32 - amount));
            carry = (value >> (amount - 1)) & 0x1;
        } else {
            result = 0xffffffff * msb;
            carry = msb;
        }
    }

    return result;
}

u32 Interpreter::alu_ror(u32 value, int amount, bool& carry, bool imm) {
    u32 result = 0;

    if (imm) {
        if (amount == 0) {
            result = (carry << 31) | (value >> 1);
            carry = value & 0x1;
        } else {
            result = common::rotate_right(value, amount);
            carry = (value >> (amount - 1)) & 0x1;
        }
    } else {
        if (amount == 0) {
            result = value;
        } else if ((amount & 0x1f) == 0) {
            result = value;
            carry = value >> 31;
        } else {
            result = common::rotate_right(value, amount & 0x1f);
            carry = (value >> ((amount & 0x1f) - 1)) & 0x1;
        }
    }

    return result;
}

} // namespace arm