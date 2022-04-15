#pragma once

#include <string>
#include "core/arm/Decoder/Decoder.h"

class Disassembler {
public:
    std::string disassemble(u32 instruction, bool arm);
    std::string disassemble_arm(u32 instruction);
    std::string disassemble_thumb(u16 instruction);
    std::string unknown_instruction(u32 instruction);

    std::string arm_branch_link_maybe_exchange(u32 instruction);
    std::string arm_branch_exchange(u32 instruction);
    std::string arm_count_leading_zeroes(u32 instruction);
    std::string arm_branch_link_exchange_register(u32 instruction);
    std::string arm_single_data_swap(u32 instruction);
    std::string arm_multiply(u32 instruction);
    std::string arm_saturating_add_subtract(u32 instruction);
    std::string arm_multiply_long(u32 instruction);
    std::string arm_halfword_data_transfer(u32 instruction);
    std::string arm_psr_transfer(u32 instruction);
    std::string arm_block_data_transfer(u32 instruction);
    std::string arm_single_data_transfer(u32 instruction);
    std::string arm_data_processing(u32 instruction);
    std::string arm_coprocessor_register_transfer(u32 instruction);
    std::string arm_software_interrupt(u32 instruction);
    std::string arm_signed_halfword_accumulate_long(u32 instruction);
    std::string arm_signed_halfword_word_multiply(u32 instruction);
    std::string arm_signed_halfword_multiply(u32 instruction);
    std::string arm_breakpoint(u32 instruction);

    std::string arm_data_processing_get_op2(u32 instruction);
    std::string arm_data_processing_regular(u32 instruction, std::string opcode);
    std::string arm_data_processing_only_destination(u32 instruction, std::string opcode);
    std::string arm_data_processing_only_operand(u32 instruction, std::string opcode);
    std::string arm_single_data_transfer_get_address(u32 instruction);


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
    std::string thumb_load_store(u32 instruction);
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

private:
    Decoder<Disassembler> decoder;
};