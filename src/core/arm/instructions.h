#pragma once

#include "common/bits.h"
#include "common/types.h"
#include "core/arm/cpu.h"

namespace core::arm {

enum class ShiftType {
    LSL = 0,
    LSR = 1,
    ASR = 2,
    ROR = 3,
};

struct ARMDataProcessing {
    enum class Opcode {
        AND = 0,
        EOR = 1,
        SUB = 2,
        RSB = 3,
        ADD = 4,
        ADC = 5,
        SBC = 6,
        RSC = 7,
        TST = 8,
        TEQ = 9,
        CMP = 10,
        CMN = 11,
        ORR = 12,
        MOV = 13,
        BIC = 14,
        MVN = 15,
    };

    static ARMDataProcessing decode(u32 instruction) {
        set_flags = common::get_bit(instruction, 20);
        imm = common::get_bit(instruction, 25);
        rd = static_cast<Reg>(common::get_field(instruction, 12, 4));
        rn = static_cast<Reg>(common::get_field(instruction, 16, 4));
        opcode = static_cast<Opcode>(common::get_field(instruction, 21, 4));
        rhs.imm.shift = common::get_field(instruction, 8, 4) * 2;
        rhs.imm.rotated = common::rotate_right(instruction & 0xff, rhs.imm.shift);
        rhs.reg.rm = static_cast<Reg>(common::get_field(instruction, 0, 4));
        rhs.reg.shift_type = static_cast<ShiftType>(common::get_field(instruction, 5, 2));
        rhs.reg.imm = !common::get_bit(instruction, 4);
        rhs.reg.amount.rs = static_cast<Reg>(common::get_field(instruction, 8, 4));
        rhs.reg.amount.imm = common::get_field(instruction, 7, 5);
    }

    bool set_flags;
    bool imm;
    Reg rd;
    Reg rn;
    Opcode opcode;

    union {
        struct {
            int shift;
            u32 rotated;
        } imm;

        struct {
            Reg rm;
            ShiftType shift_type;
            union {
                Reg rs; // shift by a value specified in a register
                u8 imm; // shift by a value specified in an immediate
            } amount;
        } reg;
    } rhs;
};

} // namespace core::arm