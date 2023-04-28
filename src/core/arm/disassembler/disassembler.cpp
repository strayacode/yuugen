#include <array>
#include "common/logger.h"
#include "core/arm/disassembler/disassembler.h"

namespace core::arm {

std::array<std::string, 16> register_names = {
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
    "r8", "r9", "r10", "r11", "r12", "sp", "lr", "pc"
}; 

std::string Disassembler::disassemble_arm(u32 instruction) {
    return (this->*(decoder.get_arm_handler(instruction)))(instruction);
}

std::string Disassembler::disassemble_thumb(u16 instruction) {
    return (this->*(decoder.get_thumb_handler(instruction)))(instruction);
}

std::string Disassembler::illegal_instruction(u32 instruction) {
    return "...";
}

std::string Disassembler::get_register_name(Reg reg) {
    return register_names[reg];
}


} // namespace core::arm