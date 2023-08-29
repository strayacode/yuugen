#pragma once

#include <string>
#include "arm/decoder.h"
#include "arm/cpu.h"

namespace arm {

class Disassembler {
public:
    Disassembler();

    std::string disassemble_arm(u32 instruction);
    std::string disassemble_thumb(u16 instruction);
    const char* get_register_name(GPR reg);
    const char* get_condition_name(Condition condition);

    // arm instruction handlers
    std::string arm_branch_link_maybe_exchange(u32 instruction);
    std::string arm_branch_exchange(u32 instruction);
    std::string arm_count_leading_zeroes(u32 instruction);
    std::string arm_branch_link(u32 instruction);
    std::string arm_branch_link_exchange(u32 instruction);
    std::string arm_branch_link_exchange_register(u32 instruction);
    std::string arm_single_data_swap(u32 instruction);
    std::string arm_multiply(u32 instruction);
    std::string arm_saturating_add_subtract(u32 instruction);
    std::string arm_multiply_long(u32 instruction);
    std::string arm_halfword_data_transfer(u32 instruction);
    std::string arm_status_load(u32 instruction);
    std::string arm_status_store_register(u32 instruction);
    std::string arm_status_store_immediate(u32 instruction);
    std::string arm_block_data_transfer(u32 instruction);
    std::string arm_single_data_transfer(u32 instruction);
    std::string arm_data_processing(u32 instruction);
    std::string arm_coprocessor_register_transfer(u32 instruction);
    std::string arm_software_interrupt(u32 instruction);
    std::string arm_signed_multiply_accumulate_long(u32 instruction);
    std::string arm_signed_multiply_word(u32 instruction);
    std::string arm_signed_multiply(u32 instruction);
    std::string arm_breakpoint(u32 instruction);

    // thumb instruction handlers
    std::string thumb_alu_immediate(u32 instruction);
    std::string thumb_branch_link_offset(u32 instruction);
    std::string thumb_branch_link_setup(u32 instruction);
    std::string thumb_branch_link_exchange_offset(u32 instruction);
    std::string thumb_branch(u32 instruction);
    std::string thumb_push_pop(u32 instruction);
    std::string thumb_data_processing_register(u32 instruction);
    std::string thumb_special_data_processing(u32 instruction);
    std::string thumb_branch_link_exchange(u32 instruction);
    std::string thumb_branch_exchange(u32 instruction);
    std::string thumb_load_store_register_offset(u32 instruction);
    std::string thumb_load_store_signed(u32 instruction);
    std::string thumb_load_pc(u32 instruction);
    std::string thumb_load_store_sp_relative(u32 instruction);
    std::string thumb_load_store_halfword(u32 instruction);
    std::string thumb_add_subtract(u32 instruction);
    std::string thumb_shift_immediate(u32 instruction);
    std::string thumb_software_interrupt(u32 instruction);
    std::string thumb_branch_conditional(u32 instruction);
    std::string thumb_load_store_multiple(u32 instruction);
    std::string thumb_load_store_immediate(u32 instruction);
    std::string thumb_add_sp_pc(u32 instruction);
    std::string thumb_adjust_stack_pointer(u32 instruction);

    // helpers
    std::string arm_data_processing_rhs(u32 instruction);

    std::string illegal_instruction(u32 instruction);

private:
    Decoder<Disassembler> decoder;

    std::array<const char*, 16> register_names = {
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
        "r8", "r9", "r10", "r11", "r12", "sp", "lr", "pc"
    }; 

    std::array<const char*, 16> condition_names = {
        "eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc",
        "hi", "ls", "ge", "lt", "gt", "le", "", "nv"
    }; 
};

} // namespace arm