#pragma once

#include <string>
#include "common/types.h"

std::string DisassembleARMBranchLinkMaybeExchange(u32 instruction) {
    if ((instruction & 0xF0000000) != 0xF0000000) {
        const bool link = (instruction >> 24) & 0x1;
        s32 offset = ((instruction & (1 << 23)) ? 0xFC000000 : 0) | ((instruction & 0xFFFFFF) << 2);

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