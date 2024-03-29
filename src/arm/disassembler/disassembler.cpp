#include <array>
#include "common/logger.h"
#include "arm/disassembler/disassembler.h"

namespace arm {

std::string Disassembler::disassemble_arm(u32 instruction) {
    return (this->*(decoder.get_arm_handler(instruction)))(instruction);
}

std::string Disassembler::disassemble_thumb(u16 instruction) {
    return (this->*(decoder.get_thumb_handler(instruction)))(instruction);
}

std::string Disassembler::illegal_instruction(u32 instruction) {
    return "...";
}

const char* Disassembler::get_register_name(GPR reg) {
    return register_names[reg];
}

} // namespace arm