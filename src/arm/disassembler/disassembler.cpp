#include <array>
#include "common/logger.h"
#include "arm/disassembler/disassembler.h"

namespace arm {

Disassembler::Disassembler() {
    register_names = {
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
        "r8", "r9", "r10", "r11", "r12", "sp", "lr", "pc"
    }; 

    condition_names = {
        "eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc",
        "hi", "ls", "ge", "lt", "gt", "le", "", "nv"
    }; 
}

std::string Disassembler::disassemble_arm(u32 instruction) {
    return (this->*(decoder.get_arm_handler(instruction)))(instruction);
}

std::string Disassembler::disassemble_thumb(u16 instruction) {
    return (this->*(decoder.get_thumb_handler(instruction)))(instruction);
}

std::string Disassembler::illegal_instruction(u32 instruction) {
    return "...";
}

const char* Disassembler::get_register_name(Reg reg) {
    return register_names[reg];
}

const char* Disassembler::get_condition_name(Condition condition) {
    return condition_names[condition];
}


} // namespace arm