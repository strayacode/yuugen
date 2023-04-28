#include "core/arm/disassembler/disassembler.h"

namespace core::arm {

std::string Disassembler::thumb_alu_immediate(u32 instruction) {
    return "handle thumb_alu_immediate";
}

std::string Disassembler::thumb_branch_link_offset(u32 instruction) {
    return "handle thumb_branch_link_offset";
}

std::string Disassembler::thumb_branch_link_setup(u32 instruction) {
    return "handle thumb_branch_link_setup";
}

std::string Disassembler::thumb_branch_link_exchange_offset(u32 instruction) {
    return "handle thumb_branch_link_exchange_offset";
}

std::string Disassembler::thumb_branch(u32 instruction) {
    return "handle thumb_branch";
}

std::string Disassembler::thumb_push_pop(u32 instruction) {
    return "handle thumb_push_pop";
}

std::string Disassembler::thumb_data_processing_register(u32 instruction) {
    return "handle thumb_data_processing_register";
}

std::string Disassembler::thumb_special_data_processing(u32 instruction) {
    return "handle thumb_special_data_processing";
}

std::string Disassembler::thumb_branch_link_exchange(u32 instruction) {
    return "handle thumb_branch_link_exchange";
}

std::string Disassembler::thumb_branch_exchange(u32 instruction) {
    return "handle thumb_branch_exchange";
}

std::string Disassembler::thumb_load_store(u32 instruction) {
    return "handle thumb_load_store";
}

std::string Disassembler::thumb_load_pc(u32 instruction) {
    return "handle thumb_load_pc";
}

std::string Disassembler::thumb_load_store_sp_relative(u32 instruction) {
    return "handle thumb_load_store_sp_relative";
}

std::string Disassembler::thumb_load_store_halfword(u32 instruction) {
    return "handle thumb_load_store_halfword";
}

std::string Disassembler::thumb_add_subtract(u32 instruction) {
    return "handle thumb_add_subtract";
}

std::string Disassembler::thumb_shift_immediate(u32 instruction) {
    return "handle thumb_shift_immediate";
}

std::string Disassembler::thumb_software_interrupt(u32 instruction) {
    return "handle thumb_software_interrupt";
}

std::string Disassembler::thumb_branch_conditional(u32 instruction) {
    return "handle thumb_branch_conditional";
}

std::string Disassembler::thumb_load_store_multiple(u32 instruction) {
    return "handle thumb_load_store_multiple";
}

std::string Disassembler::thumb_load_store_immediate(u32 instruction) {
    return "handle thumb_load_store_immediate";
}

std::string Disassembler::thumb_add_sp_pc(u32 instruction) {
    return "handle thumb_add_sp_pc";
}

std::string Disassembler::thumb_adjust_stack_pointer(u32 instruction) {
    return "handle thumb_adjust_stack_pointer";
}


} // namespace core::arm