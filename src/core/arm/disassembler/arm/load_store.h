#pragma once

#include <string>
#include "common/types.h"

std::string ARMSingleDataTransferGetAddress(u32 instruction) {
    const bool shifted_register = (instruction >> 25) & 0x1;

    if (shifted_register) {
        return "handle";
    } else {
        return format("#0x%08x", instruction & 0xFFF);
    }
}

std::string DisassembleARMSingleDataTransfer(u32 instruction) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;

    const bool writeback = (instruction >> 21) & 0x1;
    const bool up = (instruction >> 23) & 0x1;
    const bool pre = (instruction >> 24) & 0x1;
    const bool shifted_register = (instruction >> 25) & 0x1;
    
    std::string name = (instruction >> 20) & 0x1 ? "ldr" : "str";
    std::string byte = (instruction >> 22) & 0x1 ? "b" : "";
    std::string address = "";

    if (!shifted_register && (instruction & 0xFFF) == 0) {
        address = format("[r%d]", rn);
    } else {
        if (pre) {
            address = format("[r%d, %s]", rn, ARMSingleDataTransferGetAddress(instruction).c_str());
        } else {
            address = format("[r%d], %s", rn, ARMSingleDataTransferGetAddress(instruction).c_str());
        }
    }
    
    return format("%s%s r%d, %s", name.c_str(), byte.c_str(), rd, address.c_str());
}

std::string DisassembleARMHalfwordDataTransfer(u32 instruction) {
    u8 rm = instruction & 0xF;
    u8 opcode = (instruction >> 5) & 0x3;
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    const bool load = (instruction >> 20) & 0x1;

    switch (opcode) {
    case 0x1:
        if (load) {
            return "ldrh";
        } else {
            return "strh";
        }
        break;
    case 0x2:
        if (load) {
            return "ldrsb";
        } else {
            return "ldrd";
        }
        break;
    case 0x3:
        if (load) {
            return "ldrsh";
        } else {
            return "strd";
        }
        break;
    default:
        log_fatal("handle opcode %d", opcode);
    }
}