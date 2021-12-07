#pragma once

#include <string>
#include "common/types.h"

std::string DisassembleARMSingleDataTransfer(u32 instruction) {
    return "ldr/str";
}

std::string DisassembleARMHalfwordDataTransfer(u32 instruction) {
    return "ldrh/strh";
}