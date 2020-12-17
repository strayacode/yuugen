#pragma once
#include <emulator/common/types.h>
// plan is to have the disassembler separated from the ARM class to increase modularity
void disassemble_instruction(u32 opcode);