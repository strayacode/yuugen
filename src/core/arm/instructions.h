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
        opcode.condition = static_cast<Condition>(common::get_field<28, 4>(instruction));

        if (opcode.imm) {
            opcode.rhs.imm.shift = common::get_field<8, 4>(instruction) * 2;
            opcode.rhs.imm.rotated = common::rotate_right(instruction & 0xff, opcode.rhs.imm.shift);
        } else {
            opcode.rhs.reg.rm = static_cast<Reg>(common::get_field<0, 4>(instruction));
            opcode.rhs.reg.shift_type = static_cast<ShiftType>(common::get_field<5, 2>(instruction));
            opcode.rhs.reg.imm = !common::get_bit<4>(instruction);
            opcode.rhs.reg.amount.rs = static_cast<Reg>(common::get_field<8, 4>(instruction));
            opcode.rhs.reg.amount.imm = common::get_field<7, 5>(instruction);
        }

        return opcode;
    }

    bool set_flags;
    bool imm;
    Reg rd;
    Reg rn;
    Opcode opcode;
    Condition condition;

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

struct ARMStatusLoad {
    static ARMStatusLoad decode(u32 instruction) {
        ARMStatusLoad opcode;
        opcode.spsr = common::get_bit<22>(instruction);
        opcode.rd = static_cast<Reg>(common::get_field<12, 4>(instruction));
        return opcode;
    }

    bool spsr;
    Reg rd;
};

struct ARMStatusStore {
    static ARMStatusStore decode(u32 instruction) {
        ARMStatusStore opcode;
        opcode.spsr = common::get_bit<22>(instruction);
        opcode.imm = common::get_bit<25>(instruction);

        if (common::get_bit<16>(instruction)) {
            opcode.mask |= 0x000000ff;
        }
        if (common::get_bit<17>(instruction)) {
            opcode.mask |= 0x0000ff00;
        }
        if (common::get_bit<18>(instruction)) {
            opcode.mask |= 0x00ff0000;
        }
        if (common::get_bit<19>(instruction)) {
            opcode.mask |= 0xff000000;
        }

        int amount = common::get_field<8, 4>(instruction) << 1;
        opcode.rhs.rotated = common::rotate_right(instruction & 0xff, amount);
        opcode.rhs.rm = static_cast<Reg>(common::get_field<0, 4>(instruction));
        return opcode;
    }

    bool spsr;
    bool imm;
    u32 mask;
    
    union {
        u32 rotated;
        Reg rm;
    } rhs;
};

struct ARMBlockDataTransfer {
    static ARMBlockDataTransfer decode(u32 instruction) {
        ARMBlockDataTransfer opcode;
        opcode.rlist = common::get_field<0, 16>(instruction);
        opcode.r15_in_rlist = common::get_bit<15>(instruction);
        opcode.load = common::get_bit<20>(instruction);
        opcode.writeback = common::get_bit<21>(instruction);
        opcode.psr = common::get_bit<22>(instruction);
        opcode.up = common::get_bit<23>(instruction);
        opcode.pre = common::get_bit<24>(instruction);
        opcode.rn = static_cast<Reg>(common::get_field<16, 4>(instruction));
        return opcode;
    }

    u16 rlist;
    bool r15_in_rlist;
    bool load;
    bool writeback;
    bool psr;
    bool up;
    bool pre;
    Reg rn;
};

struct ARMSingleDataTransfer {
    static ARMSingleDataTransfer decode(u32 instruction) {
        ARMSingleDataTransfer opcode;
        opcode.load = common::get_bit<20>(instruction);
        opcode.writeback = common::get_bit<21>(instruction);
        opcode.byte = common::get_bit<22>(instruction);
        opcode.up = common::get_bit<23>(instruction);
        opcode.pre = common::get_bit<24>(instruction);
        opcode.imm = !common::get_bit<25>(instruction);
        opcode.rd = static_cast<Reg>(common::get_field<12, 4>(instruction));
        opcode.rn = static_cast<Reg>(common::get_field<16, 4>(instruction));
        opcode.rhs.imm = common::get_field<0, 12>(instruction);
        opcode.rhs.reg.rm = static_cast<Reg>(common::get_field<0, 4>(instruction));
        opcode.rhs.reg.shift_type = static_cast<ShiftType>(common::get_field<5, 2>(instruction));
        opcode.rhs.reg.amount = common::get_field<7, 5>(instruction);
        return opcode;
    }

    bool load;
    bool writeback;
    bool byte;
    bool up;
    bool pre;
    bool imm;
    Reg rd;
    Reg rn;

    union {
        u32 imm;
        struct {
            Reg rm;
            ShiftType shift_type;
            int amount;
        } reg;
    } rhs;
};

struct ARMCoprocessorRegisterTransfer {
    static ARMCoprocessorRegisterTransfer decode(u32 instruction) {
        ARMCoprocessorRegisterTransfer opcode;
        opcode.crm = static_cast<Reg>(common::get_field<0, 4>(instruction));
        opcode.crn = static_cast<Reg>(common::get_field<16, 4>(instruction));
        opcode.cp = common::get_field<5, 3>(instruction);
        opcode.rd = static_cast<Reg>(common::get_field<12, 4>(instruction));
        opcode.load = common::get_bit<20>(instruction);
        return opcode;
    }

    Reg crm;
    Reg crn;
    u8 cp;
    Reg rd;
    bool load;
};

struct ThumbAddSubtract {
    static ThumbAddSubtract decode(u16 instruction) {
        ThumbAddSubtract opcode;
        opcode.rd = static_cast<Reg>(common::get_field<0, 3>(instruction));
        opcode.rs = static_cast<Reg>(common::get_field<3, 3>(instruction));
        opcode.rn = static_cast<Reg>(common::get_field<6, 3>(instruction));
        opcode.sub = common::get_bit<9>(instruction);
        opcode.imm = common::get_bit<10>(instruction);
        return opcode;
    }

    Reg rd;
    Reg rs;
    Reg rn;
    bool sub;
    bool imm;
};

struct ThumbShiftImmediate {
    static ThumbShiftImmediate decode(u16 instruction) {
        ThumbShiftImmediate opcode;
        opcode.rd = static_cast<Reg>(common::get_field<0, 3>(instruction));
        opcode.rs = static_cast<Reg>(common::get_field<3, 3>(instruction));
        opcode.amount = common::get_field<6, 5>(instruction);
        opcode.shift_type = static_cast<ShiftType>(common::get_field<11, 2>(instruction));
        return opcode;
    }

    Reg rd;
    Reg rs;
    int amount;
    ShiftType shift_type;
};

struct ThumbALUImmediate {
    enum Opcode {
        MOV,
        CMP,
        ADD,
        SUB,
    };

    static ThumbALUImmediate decode(u16 instruction) {
        ThumbALUImmediate opcode;
        opcode.imm = common::get_field<0, 8>(instruction);
        opcode.rd = static_cast<Reg>(common::get_field<8, 3>(instruction));
        opcode.opcode = static_cast<Opcode>(common::get_field<11, 2>(instruction));
        return opcode;
    }

    u8 imm;
    Reg rd;
    Opcode opcode;
};

struct ThumbDataProcessing {
    static ThumbDataProcessing decode(u16 instruction) {
        ThumbDataProcessing opcode;
        return opcode;
    }

};

struct ThumbDataProcessingSpecial {
    static ThumbDataProcessingSpecial decode(u16 instruction) {
        ThumbDataProcessingSpecial opcode;
        return opcode;
    }

};

struct ThumbAdjustStackPointer {
    static ThumbAdjustStackPointer decode(u16 instruction) {
        ThumbAdjustStackPointer opcode;
        opcode.imm = common::get_field<0, 7>(instruction) << 2;
        opcode.sub = common::get_bit<7>(instruction);
        return opcode;
    }

    u32 imm;
    bool sub;
};

struct ThumbAddSPPC {
    static ThumbAddSPPC decode(u16 instruction) {
        ThumbAddSPPC opcode;
        opcode.imm = common::get_field<0, 8>(instruction) << 2;
        opcode.rd = static_cast<Reg>(common::get_field<8, 3>(instruction));
        opcode.sp = common::get_bit<11>(instruction);
        return opcode;
    }

    u32 imm;
    Reg rd;
    bool sp;
};

struct ThumbBranchExchange {
    static ThumbBranchExchange decode(u16 instruction) {
        ThumbBranchExchange opcode;
        opcode.rm = static_cast<Reg>(common::get_field<3, 4>(instruction));
        return opcode;
    }

    Reg rm;
};

struct ThumbBranchLinkExchange {
    static ThumbBranchLinkExchange decode(u16 instruction) {
        ThumbBranchLinkExchange opcode;
        return opcode;
    }

};

struct ThumbBranchLinkSetup {
    static ThumbBranchLinkSetup decode(u16 instruction) {
        ThumbBranchLinkSetup opcode;
        return opcode;
    }

};

struct ThumbBranchLinkOffset {
    static ThumbBranchLinkOffset decode(u16 instruction) {
        ThumbBranchLinkOffset opcode;
        return opcode;
    }

};

struct ThumbBranchLinkExchangeOffset {
    static ThumbBranchLinkExchangeOffset decode(u16 instruction) {
        ThumbBranchLinkExchangeOffset opcode;
        return opcode;
    }

};

struct ThumbBranch {
    static ThumbBranch decode(u16 instruction) {
        ThumbBranch opcode;
        opcode.offset = common::sign_extend<s32, 10>(instruction) | (common::get_field<0, 11>(instruction) << 1);
        return opcode;
    }

    u32 offset;
};

struct ThumbBranchConditional {
    static ThumbBranchConditional decode(u16 instruction) {
        ThumbBranchConditional opcode;
        return opcode;
    }
};

struct ThumbLoadPC {
    static ThumbLoadPC decode(u16 instruction) {
        ThumbLoadPC opcode;
        opcode.imm = common::get_field<0, 8>(instruction) << 2;
        opcode.rd = static_cast<Reg>(common::get_field<8, 3>(instruction));
        return opcode;
    }

    u32 imm;
    Reg rd;
};

struct ThumbLoadStore {
    static ThumbLoadStore decode(u16 instruction) {
        ThumbLoadStore opcode;
        return opcode;
    }

};

struct ThumbLoadStoreImmediate {
    static ThumbLoadStoreImmediate decode(u16 instruction) {
        ThumbLoadStoreImmediate opcode;
        return opcode;
    }

};

struct ThumbPushPop {
    static ThumbPushPop decode(u16 instruction) {
        ThumbPushPop opcode;
        return opcode;
    }

};

struct ThumbLoadStoreSPRelative {
    static ThumbLoadStoreSPRelative decode(u16 instruction) {
        ThumbLoadStoreSPRelative opcode;
        return opcode;
    }

};

struct ThumbLoadStoreHalfword {
    static ThumbLoadStoreHalfword decode(u16 instruction) {
        ThumbLoadStoreHalfword opcode;
        opcode.rd = static_cast<Reg>(common::get_field<0, 3>(instruction));
        opcode.rn = static_cast<Reg>(common::get_field<3, 3>(instruction));
        opcode.imm = common::get_field<6, 5>(instruction);
        opcode.load = common::get_bit<11>(instruction);
        return opcode;
    }

    Reg rd;
    Reg rn;
    u32 imm;
    bool load;
};

struct ThumbLoadStoreMultiple {
    static ThumbLoadStoreMultiple decode(u16 instruction) {
        ThumbLoadStoreMultiple opcode;
        return opcode;
    }

};

} // namespace core::arm