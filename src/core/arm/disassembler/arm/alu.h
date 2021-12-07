#pragma once

#include <string>
#include "common/types.h"
#include "common/format.h"
#include "common/arithmetic.h"

std::string ARMDataProcessingGetOp2(u32 instruction) {
    const u8 shift_imm = (instruction >> 25) & 0x1;

    if (shift_imm) {
        u32 immediate = instruction & 0xFF;
        u8 shift_amount = ((instruction >> 8) & 0xF) << 1;

        return format("#0x%08x", rotate_right(immediate, shift_amount));
    } else {
        u8 rm = instruction & 0xF;
        
        std::string shift_types[4] = {"lsl", "lsr", "asr", "ror"};
        std::string shift_type = shift_types[(instruction >> 5) & 0x3];

        bool immediate = !(instruction & (1 << 4));

        if (immediate) {
            u8 shift_amount = (instruction >> 7) & 0x1F;

            if (shift_amount) {
                return format("r%d", rm);
            }

            return format("r%d %s #0x%02x", rm, shift_type.c_str(), shift_amount);
        } else {
            u8 rs = (instruction >> 8) & 0xF;
            
            return format("r%d %s r%d", rm, shift_type.c_str(), rs);
        }
    }
}

std::string DisassembleARMDataProcessingRegular(u32 instruction, std::string opcode) {
    std::string set_flags = (instruction >> 20) & 0x1 ? "s" : "";
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;

    return format("%s%s r%d, r%d, %s", opcode.c_str(), set_flags.c_str(), rd, rn, ARMDataProcessingGetOp2(instruction).c_str());
}

std::string DisassembleARMDataProcessingOnlyDestination(u32 instruction, std::string opcode) {
    std::string set_flags = (instruction >> 20) & 0x1 ? "s" : "";
    u8 rd = (instruction >> 12) & 0xF;

    return format("%s%s r%d, %s", opcode.c_str(), set_flags.c_str(), rd, ARMDataProcessingGetOp2(instruction).c_str());
}

std::string DisassembleARMDataProcessingOnlyOperand(u32 instruction, std::string opcode) {
    u8 rn = (instruction >> 16) & 0xF;

    return format("%s r%d, %s", opcode.c_str(), rn, ARMDataProcessingGetOp2(instruction).c_str());
}

std::string DisassembleARMDataProcessing(u32 instruction) {
    u8 opcode = (instruction >> 21) & 0xF;

    switch (opcode) {
    case 0x0:
        return DisassembleARMDataProcessingRegular(instruction, "and");
    case 0x1:
        return DisassembleARMDataProcessingRegular(instruction, "eor");
    case 0x2:
        return DisassembleARMDataProcessingRegular(instruction, "sub");
    case 0x3:
        return DisassembleARMDataProcessingRegular(instruction, "rsb");
    case 0x4:
        return DisassembleARMDataProcessingRegular(instruction, "add");
    case 0x5:
        return DisassembleARMDataProcessingRegular(instruction, "adc");
    case 0x6:
        return DisassembleARMDataProcessingRegular(instruction, "sbc");
    case 0x7:
        return DisassembleARMDataProcessingRegular(instruction, "rsc");
    case 0x8:
        return DisassembleARMDataProcessingOnlyOperand(instruction, "tst");
    case 0x9:
        return DisassembleARMDataProcessingOnlyOperand(instruction, "teq");
    case 0xA:
        return DisassembleARMDataProcessingOnlyOperand(instruction, "cmp");
    case 0xB:
        return DisassembleARMDataProcessingOnlyOperand(instruction, "cmn");
    case 0xC:
        return DisassembleARMDataProcessingRegular(instruction, "orr");
    case 0xD:
        return DisassembleARMDataProcessingOnlyDestination(instruction, "mov");
    case 0xE:
        return DisassembleARMDataProcessingRegular(instruction, "bic");
    case 0xF:
        return DisassembleARMDataProcessingOnlyDestination(instruction, "mvn");
    default:
        return "";
    }
}