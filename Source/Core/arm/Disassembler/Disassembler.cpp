#include "Core/ARM/Disassembler/Disassembler.h"

std::string Disassembler::disassemble(u32 instruction, bool arm) {
    if (arm) {
        return disassemble_arm(instruction);
    } else {
        return disassemble_thumb(instruction);
    }
}

std::string Disassembler::disassemble_arm(u32 instruction) {
    return (this->*(decoder.decode_arm(instruction)))(instruction);
}

std::string Disassembler::disassemble_thumb(u16 instruction) {
    return (this->*(decoder.decode_thumb(instruction)))(instruction);
}

std::string Disassembler::unknown_instruction(u32 instruction) {
    return "...";
}