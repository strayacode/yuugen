#pragma once

#include <string>
#include "common/types.h"
#include "common/format.h"

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
    const bool writeback = (instruction >> 21) & 0x1;
    const bool immediate = (instruction >> 22) & 0x1;
    const bool up = (instruction >> 23) & 0x1;
    const bool pre = (instruction >> 24) & 0x1;

    std::string address = format(" [r%d%s, ", rn, pre ? "" : "]");

    if (immediate) {
        address += format("#0x%08x", ((instruction >> 4) & 0xF0) | (instruction & 0xF));
    } else {
        address += format("r%d", rm);
    }

    if (pre) {
        address += "]";
    }

    if (writeback) {
        address += "!";
    }

    switch (opcode) {
    case 0x1:
        if (load) {
            return "ldrh" + address;
        } else {
            return "strh" + address;
        }
        break;
    case 0x2:
        if (load) {
            return "ldrsb" + address;
        } else {
            return "ldrd" + address;
        }
        break;
    case 0x3:
        if (load) {
            return "ldrsh" + address;
        } else {
            return "strd" + address;
        }
        break;
    default:
        log_fatal("handle opcode %d", opcode);
    }
}

std::string DisassembleARMBlockDataTransfer(u32 instruction) {
    const bool load = (instruction >> 20) & 0x1;
    const bool writeback = (instruction >> 21) & 0x1;
    const bool load_psr = (instruction >> 22) & 0x1;
    const bool up = (instruction >> 23) & 0x1;
    const bool pre = (instruction >> 24) & 0x1;
    u8 rn = (instruction >> 16) & 0xF;

    int highest_bit = 0;

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            highest_bit = i;
        }
    }

    std::string regs_list = "";

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            regs_list += format("r%d", i);

            if (i != highest_bit) {
                regs_list += ", ";
            }
        }
    }

    if (load) {
        return format("ldm r%d, {%s}", rn, regs_list.c_str());
    } else {
        return format("stm r%d, {%s}", rn, regs_list.c_str());
    }
}

std::string DisassembleARMStatusTransfer(u32 instruction) {
    const bool opcode = (instruction >> 21) & 0x1;
    const bool spsr = (instruction >> 22) & 0x1;
    u8 rm = instruction & 0xF;

    if (opcode) {
        // msr
        u8 immediate = (instruction >> 25) & 0x1;

        std::string mask = "";

        if (instruction & (1 << 19)) {
            mask += "f";
        }

        if (instruction & (1 << 18)) {
            mask += "s";
        }

        if (instruction & (1 << 17)) {
            mask += "x";
        }

        if (instruction & (1 << 16)) {
            mask += "c";
        }
        
        if (immediate) {
            u32 immediate = instruction & 0xFF;
            u8 rotate_amount = ((instruction >> 8) & 0xF) << 1;

            return format("msr %s, #0x%08x", mask.c_str(), rotate_right(immediate, rotate_amount));
        } else {
            return format("msr %s, r%d", mask.c_str(), rm);
        }
    } else {
        // mrs
        u8 rd = (instruction >> 12) & 0xF;

        std::string psr = spsr ? "spsr" : "cpsr";

        return format("mrs r%d, %s", rd, psr.c_str());
    }
}

std::string DisassembleARMCoprocessorRegisterTransfer(u32 instruction) {
    const bool opcode = (instruction >> 20) & 0x1;

    if (opcode) {
        return "mrc";
    } else {
        return "mcr";
    }
}