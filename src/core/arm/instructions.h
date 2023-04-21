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
        ARMDataProcessing opcode;
        opcode.set_flags = common::get_bit<20>(instruction);
        opcode.imm = common::get_bit<25>(instruction);
        opcode.rd = static_cast<Reg>(common::get_field<12, 4>(instruction));
        opcode.rn = static_cast<Reg>(common::get_field<16, 4>(instruction));
        opcode.opcode = static_cast<Opcode>(common::get_field<21, 4>(instruction));
        opcode.rhs.imm.shift = common::get_field<8, 4>(instruction) * 2;
        opcode.rhs.imm.rotated = common::rotate_right(instruction & 0xff, opcode.rhs.imm.shift);
        opcode.rhs.reg.rm = static_cast<Reg>(common::get_field<0, 4>(instruction));
        opcode.rhs.reg.shift_type = static_cast<ShiftType>(common::get_field<5, 2>(instruction));
        opcode.rhs.reg.imm = !common::get_bit<4>(instruction);
        opcode.rhs.reg.amount.rs = static_cast<Reg>(common::get_field<8, 4>(instruction));
        opcode.rhs.reg.amount.imm = common::get_field<7, 5>(instruction);
        return opcode;
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
            bool imm;
            union {
                Reg rs; // shift by a value specified in a register
                u8 imm; // shift by a value specified in an immediate
            } amount;
        } reg;
    } rhs;
};

struct ARMMultiply {
    static ARMMultiply decode(u32 instruction) {
        ARMMultiply opcode;
        opcode.set_flags = common::get_bit<20>(instruction);
        opcode.accumulate = common::get_bit<21>(instruction);
        opcode.rm = static_cast<Reg>(common::get_field<0, 4>(instruction));
        opcode.rs = static_cast<Reg>(common::get_field<8, 4>(instruction));
        opcode.rn = static_cast<Reg>(common::get_field<12, 4>(instruction));
        opcode.rd = static_cast<Reg>(common::get_field<16, 4>(instruction));
        return opcode;
    }

    bool set_flags;
    bool accumulate;
    Reg rm;
    Reg rs;
    Reg rn;
    Reg rd;
};

struct ARMMultiplyLong {
    static ARMMultiplyLong decode(u32 instruction) {
        ARMMultiplyLong opcode;
        opcode.set_flags = common::get_bit<20>(instruction);
        opcode.accumulate = common::get_bit<21>(instruction);
        opcode.sign = common::get_bit<22>(instruction);
        opcode.rm = static_cast<Reg>(common::get_field<0, 4>(instruction));
        opcode.rs = static_cast<Reg>(common::get_field<8, 4>(instruction));
        opcode.rdlo = static_cast<Reg>(common::get_field<12, 4>(instruction));
        opcode.rdhi = static_cast<Reg>(common::get_field<16, 4>(instruction));
        return opcode;
    }

    bool set_flags;
    bool accumulate;
    bool sign;
    Reg rm;
    Reg rs;
    Reg rdlo;
    Reg rdhi;
};

struct ARMSingleDataSwap {
    static ARMSingleDataSwap decode(u32 instruction) {
        ARMSingleDataSwap opcode;
        opcode.rm = static_cast<Reg>(common::get_field<0, 4>(instruction));
        opcode.rd = static_cast<Reg>(common::get_field<12, 4>(instruction));
        opcode.rn = static_cast<Reg>(common::get_field<16, 4>(instruction));
        opcode.byte = common::get_bit<22>(instruction);
        return opcode;
    }

    Reg rm;
    Reg rd;
    Reg rn;
    bool byte;
};

struct ARMCountLeadingZeroes {
    static ARMCountLeadingZeroes decode(u32 instruction) {
        ARMCountLeadingZeroes opcode;
        opcode.rm = static_cast<Reg>(common::get_field<0, 4>(instruction));
        opcode.rd = static_cast<Reg>(common::get_field<12, 4>(instruction));
        return opcode;
    }

    Reg rm;
    Reg rd;
};

struct ARMSaturatingAddSubtract {
    static ARMSaturatingAddSubtract decode(u32 instruction) {
        ARMSaturatingAddSubtract opcode;
        opcode.rm = static_cast<Reg>(common::get_field<0, 4>(instruction));
        opcode.rd = static_cast<Reg>(common::get_field<12, 4>(instruction));
        opcode.rn = static_cast<Reg>(common::get_field<16, 4>(instruction));
        opcode.sub = common::get_bit<21>(instruction);
        opcode.double_rhs = common::get_bit<22>(instruction);
        return opcode;
    }

    Reg rm;
    Reg rd;
    Reg rn;
    bool double_rhs;
    bool sub;
};

struct ARMSignedMultiply {
    static ARMSignedMultiply decode(u32 instruction) {
        ARMSignedMultiply opcode;
        opcode.rm = static_cast<Reg>(common::get_field<0, 4>(instruction));
        opcode.rs = static_cast<Reg>(common::get_field<8, 4>(instruction));
        opcode.rn = static_cast<Reg>(common::get_field<12, 4>(instruction));
        opcode.rd = static_cast<Reg>(common::get_field<16, 4>(instruction));
        opcode.accumulate = common::get_field<21, 3>(instruction) == 0;
        opcode.x = common::get_bit<5>(instruction);
        opcode.y = common::get_bit<6>(instruction);
        return opcode;
    }

    Reg rm;
    Reg rs;
    Reg rn;
    Reg rd;
    bool accumulate;
    bool x;
    bool y;
};

struct ARMSignedMultiplyWord {
    static ARMSignedMultiplyWord decode(u32 instruction){
        ARMSignedMultiplyWord opcode;
        opcode.rm = static_cast<Reg>(common::get_field<0, 4>(instruction));
        opcode.rs = static_cast<Reg>(common::get_field<8, 4>(instruction));
        opcode.rn = static_cast<Reg>(common::get_field<12, 4>(instruction));
        opcode.rd = static_cast<Reg>(common::get_field<16, 4>(instruction));
        opcode.accumulate = !common::get_bit<5>(instruction);
        opcode.y = common::get_bit<6>(instruction);
        return opcode;
    }

    Reg rm;
    Reg rs;
    Reg rn;
    Reg rd;
    bool accumulate;
    bool y;
};

struct ARMSignedMultiplyAccumulateLong {
    static ARMSignedMultiplyAccumulateLong decode(u32 instruction){
        ARMSignedMultiplyAccumulateLong opcode;
        opcode.rm = static_cast<Reg>(common::get_field<0, 4>(instruction));
        opcode.rs = static_cast<Reg>(common::get_field<8, 4>(instruction));
        opcode.rn = static_cast<Reg>(common::get_field<12, 4>(instruction));
        opcode.rd = static_cast<Reg>(common::get_field<16, 4>(instruction));
        opcode.x = common::get_bit<5>(instruction);
        opcode.y = common::get_bit<6>(instruction);
        return opcode;
    }

    Reg rm;
    Reg rs;
    Reg rn;
    Reg rd;
    bool x;
    bool y;
};

struct ARMBranchExchange {
    static ARMBranchExchange decode(u32 instruction) {
        ARMBranchExchange opcode;
        opcode.rm = static_cast<Reg>(common::get_field<0, 4>(instruction));
        return opcode;
    }

    Reg rm;
};

struct ARMBranchLink {
    static ARMBranchLink decode(u32 instruction) {
        ARMBranchLink opcode;
        opcode.link = common::get_bit<24>(instruction);
        opcode.offset = common::sign_extend<s32, 24>(common::get_field<0, 24>(instruction)) << 2;
        return opcode;
    }

    bool link;
    u32 offset;
};

struct ARMBranchLinkExchange {
    static ARMBranchLinkExchange decode(u32 instruction) {
        ARMBranchLinkExchange opcode;
        opcode.offset = (common::sign_extend<s32, 24>(common::get_field<0, 24>(instruction)) << 2) | (common::get_bit<24>(instruction) << 1);
        return opcode;
    }

    u32 offset;
};

struct ARMHalfwordDataTransfer {
    static ARMHalfwordDataTransfer decode(u32 instruction) {
        ARMHalfwordDataTransfer opcode;
        opcode.load = common::get_bit<20>(instruction);
        opcode.writeback = common::get_bit<21>(instruction);
        opcode.imm = common::get_bit<22>(instruction);
        opcode.up = common::get_bit<23>(instruction);
        opcode.pre = common::get_bit<24>(instruction);
        opcode.half = common::get_bit<5>(instruction);
        opcode.sign = common::get_bit<6>(instruction);
        opcode.rd = static_cast<Reg>(common::get_field<12, 4>(instruction));
        opcode.rn = static_cast<Reg>(common::get_field<16, 4>(instruction));
        opcode.rhs.imm = ((instruction >> 4) & 0xf0) | (instruction & 0xf);
        opcode.rhs.rm = static_cast<Reg>(common::get_field<0, 4>(instruction));
        return opcode;
    }

    bool load;
    bool writeback;
    bool imm;
    bool up;
    bool pre;
    bool half;
    bool sign;
    Reg rd;
    Reg rn;

    union {
        u32 imm;
        Reg rm;
    } rhs;
};

} // namespace core::arm