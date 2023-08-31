#pragma once

#include "arm/jit/basic_block.h"
#include "arm/jit/ir/emitter.h"

namespace arm {

class Jit;

class Translator {
public:
    Translator(Jit& jit);

    void translate(BasicBlock& basic_block);

    // arm instruction handlers
    void arm_branch_link_maybe_exchange(Emitter& emitter);
    void arm_branch_exchange(Emitter& emitter);
    void arm_count_leading_zeroes(Emitter& emitter);
    void arm_branch_link(Emitter& emitter);
    void arm_branch_link_exchange(Emitter& emitter);
    void arm_branch_link_exchange_register(Emitter& emitter);
    void arm_single_data_swap(Emitter& emitter);
    void arm_multiply(Emitter& emitter);
    void arm_saturating_add_subtract(Emitter& emitter);
    void arm_multiply_long(Emitter& emitter);
    void arm_halfword_data_transfer(Emitter& emitter);
    void arm_status_load(Emitter& emitter);
    void arm_status_store_register(Emitter& emitter);
    void arm_status_store_immediate(Emitter& emitter);
    void arm_block_data_transfer(Emitter& emitter);
    void arm_single_data_transfer(Emitter& emitter);
    void arm_data_processing(Emitter& emitter);
    void arm_coprocessor_register_transfer(Emitter& emitter);
    void arm_software_interrupt(Emitter& emitter);
    void arm_signed_multiply_accumulate_long(Emitter& emitter);
    void arm_signed_multiply_word(Emitter& emitter);
    void arm_signed_multiply(Emitter& emitter);
    void arm_breakpoint(Emitter& emitter);

    // thumb instruction handlers
    void thumb_alu_immediate(Emitter& emitter);
    void thumb_branch_link_offset(Emitter& emitter);
    void thumb_branch_link_setup(Emitter& emitter);
    void thumb_branch_link_exchange_offset(Emitter& emitter);
    void thumb_branch(Emitter& emitter);
    void thumb_push_pop(Emitter& emitter);
    void thumb_data_processing_register(Emitter& emitter);
    void thumb_special_data_processing(Emitter& emitter);
    void thumb_branch_link_exchange(Emitter& emitter);
    void thumb_branch_exchange(Emitter& emitter);
    void thumb_load_store_register_offset(Emitter& emitter);
    void thumb_load_store_signed(Emitter& emitter);
    void thumb_load_pc(Emitter& emitter);
    void thumb_load_store_sp_relative(Emitter& emitter);
    void thumb_load_store_halfword(Emitter& emitter);
    void thumb_add_subtract(Emitter& emitter);
    void thumb_shift_immediate(Emitter& emitter);
    void thumb_software_interrupt(Emitter& emitter);
    void thumb_branch_conditional(Emitter& emitter);
    void thumb_load_store_multiple(Emitter& emitter);
    void thumb_load_store_immediate(Emitter& emitter);
    void thumb_add_sp_pc(Emitter& emitter);
    void thumb_adjust_stack_pointer(Emitter& emitter);

    void illegal_instruction(Emitter& emitter);

private:
    u16 code_read_half(u32 addr);
    u32 code_read_word(u32 addr);

    u32 instruction{0};
    Jit& jit;
};

} // namespace arm