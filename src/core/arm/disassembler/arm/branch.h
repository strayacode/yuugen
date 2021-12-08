#pragma once

#include <string>
#include "common/types.h"
#include "common/format.h"

std::string DisassembleARMBranchLinkMaybeExchange(u32 instruction, u32 pc) {
    if ((instruction & 0xF0000000) != 0xF0000000) {
        const bool link = (instruction >> 24) & 0x1;
        s32 offset = pc + (((instruction & (1 << 23)) ? 0xFC000000 : 0) | ((instruction & 0xFFFFFF) << 2)) + 8;

        if (link) {
            return format("bl #0x%08x", offset);
        } else {
            return format("b #0x%08x", offset);
        }

    } else {
        s32 offset = (((instruction & (1 << 23)) ? 0xFC000000: 0) | ((instruction & 0xFFFFFF) << 2)) + ((instruction & (1 << 24)) >> 23);
        return format("blx #0x%08x", offset);
    }
}

std::string DisassembleARMBranchExchange(u32 instruction) {
    u8 rm = instruction & 0xF;
    return format("bx r%d", rm);
}

std::string DisassembleARMSoftwareInterrupt(u32 instruction) {
    u32 comment_field = instruction & 0xFFFFFF;

    return format("swi #0x%08x", comment_field);
}

std::string DisassembleARMBranchLinkExchangeRegister(u32 instruction) {
    u8 rm = instruction & 0xF;
    return format("blx r%d", rm);
}