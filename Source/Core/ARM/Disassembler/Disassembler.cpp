#include <array>
#include "Core/ARM/Disassembler/Disassembler.h"

std::array<std::string, 16> s_register_names = {
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
    "r8", "r9", "r10", "r11", "r12", "sp", "lr", "pc"
};

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

std::string Disassembler::register_name(int reg) {
    return s_register_names[reg];
}