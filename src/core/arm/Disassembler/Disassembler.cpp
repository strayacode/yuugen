#include "core/arm/Disassembler/Disassembler.h"

std::string Disassembler::disassemble(u32 instruction, u32 pc, bool arm) {
    if (arm) {
        return disassemble_arm(instruction, pc);
    } else {
        return disassemble_thumb(instruction, pc);
    }
}

std::string Disassembler::disassemble_arm(u32 instruction, u32 pc) {
    return decoder.decode_arm(instruction, pc);
}

std::string Disassembler::disassemble_thumb(u16 instruction, u32 pc) {
    return decoder.decode_thumb(instruction, pc);
}

std::string Disassembler::unknown_instruction(u32 instruction, u32 pc) {
    return "...";
}