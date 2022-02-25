#pragma once

#include <string>
#include "core/arm/decoder/decoder.h"

class Disassembler {
public:
    std::string disassemble(u32 instruction, u32 pc, bool arm);
    std::string disassemble_arm(u32 instruction, u32 pc);
    std::string disassemble_thumb(u16 instruction, u32 pc);
    std::string unknown_instruction(u32 instruction, u32 pc);

private:
    Decoder<Disassembler> decoder;
};