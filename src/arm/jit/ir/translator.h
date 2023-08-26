#pragma once

#include "arm/jit/basic_block.h"

namespace arm {

class Jit;

class Translator {
public:
    Translator(Jit& jit);

    void translate(BasicBlock& basic_block);

    // arm instruction handlers
    void arm_branch_link_maybe_exchange();
    void arm_branch_exchange();
    void arm_count_leading_zeroes();
    void arm_branch_link();
    void arm_branch_link_exchange();
    void arm_branch_link_exchange_register();
    void arm_single_data_swap();
    void arm_multiply();
    void arm_saturating_add_subtract();
    void arm_multiply_long();
    void arm_halfword_data_transfer();
    void arm_status_load();
    void arm_status_store_register();
    void arm_status_store_immediate();
    void arm_block_data_transfer();
    void arm_single_data_transfer();
    void arm_data_processing();
    void arm_coprocessor_register_transfer();
    void arm_software_interrupt();
    void arm_signed_multiply_accumulate_long();
    void arm_signed_multiply_word();
    void arm_signed_multiply();
    void arm_breakpoint();

    // thumb instruction handlers
    void thumb_alu_immediate();
    void thumb_branch_link_offset();
    void thumb_branch_link_setup();
    void thumb_branch_link_exchange_offset();
    void thumb_branch();
    void thumb_push_pop();
    void thumb_data_processing_register();
    void thumb_special_data_processing();
    void thumb_branch_link_exchange();
    void thumb_branch_exchange();
    void thumb_load_store_register_offset();
    void thumb_load_store_signed();
    void thumb_load_pc();
    void thumb_load_store_sp_relative();
    void thumb_load_store_halfword();
    void thumb_add_subtract();
    void thumb_shift_immediate();
    void thumb_software_interrupt();
    void thumb_branch_conditional();
    void thumb_load_store_multiple();
    void thumb_load_store_immediate();
    void thumb_add_sp_pc();
    void thumb_adjust_stack_pointer();

    void illegal_instruction();

private:
    u16 code_read_half(u32 addr);
    u32 code_read_word(u32 addr);

    u32 instruction;
    Jit& jit;
};

} // namespace arm