#pragma once

#include <string>
#include "common/types.h"

class Disassembler {
public:
    std::string DisassembleARMInstruction(u32 instruction);
    std::string DisassembleThumbInstruction(u16 instruction);
    std::string UnimplementedInstruction(u32 instruction);

private:
};