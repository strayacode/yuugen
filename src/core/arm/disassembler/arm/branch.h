#pragma once

#include <string>
#include "common/types.h"

std::string DisassembleARMBranchLinkMaybeExchange(u32 instruction) {
    return "blx/bx";
}