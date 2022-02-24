#pragma once

#include <string>
#include "common/types.h"
#include "core/arm/disassembler/arm/alu.h"
#include "core/arm/disassembler/arm/branch.h"
#include "core/arm/disassembler/arm/load_store.h"

std::string DisassembleARMInstruction(u32 instruction, u32 pc);
std::string DisassembleThumbInstruction(u16 instruction, u32 pc);