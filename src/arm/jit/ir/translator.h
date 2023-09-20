#pragma once

#include "arm/cpu.h"
#include "arm/instructions.h"
#include "arm/jit/basic_block.h"
#include "arm/jit/ir/ir_emitter.h"

namespace arm {

class Jit;

class Translator {
public:
    Translator(Jit& jit, IREmitter& ir);

    void translate();

    enum class BlockStatus {
        Break,
        Continue,
    };

    // arm instruction handlers
    BlockStatus arm_branch_link_maybe_exchange();
    BlockStatus arm_branch_exchange();
    BlockStatus arm_count_leading_zeroes();
    BlockStatus arm_branch_link();
    BlockStatus arm_branch_link_exchange();
    BlockStatus arm_branch_link_exchange_register();
    BlockStatus arm_single_data_swap();
    BlockStatus arm_multiply();
    BlockStatus arm_saturating_add_subtract();
    BlockStatus arm_multiply_long();
    BlockStatus arm_halfword_data_transfer();
    BlockStatus arm_status_load();
    BlockStatus arm_status_store_register();
    BlockStatus arm_status_store_immediate();
    BlockStatus arm_block_data_transfer();
    BlockStatus arm_single_data_transfer();
    BlockStatus arm_data_processing();
    BlockStatus arm_coprocessor_register_transfer();
    BlockStatus arm_software_interrupt();
    BlockStatus arm_signed_multiply_accumulate_long();
    BlockStatus arm_signed_multiply_word();
    BlockStatus arm_signed_multiply();
    BlockStatus arm_breakpoint();

    // thumb instruction handlers
    BlockStatus thumb_alu_immediate();
    BlockStatus thumb_branch_link_offset();
    BlockStatus thumb_branch_link_setup();
    BlockStatus thumb_branch_link_exchange_offset();
    BlockStatus thumb_branch();
    BlockStatus thumb_push_pop();
    BlockStatus thumb_data_processing_register();
    BlockStatus thumb_special_data_processing();
    BlockStatus thumb_branch_link_exchange();
    BlockStatus thumb_branch_exchange();
    BlockStatus thumb_load_store_register_offset();
    BlockStatus thumb_load_store_signed();
    BlockStatus thumb_load_pc();
    BlockStatus thumb_load_store_sp_relative();
    BlockStatus thumb_load_store_halfword();
    BlockStatus thumb_add_subtract();
    BlockStatus thumb_shift_immediate();
    BlockStatus thumb_software_interrupt();
    BlockStatus thumb_branch_conditional();
    BlockStatus thumb_load_store_multiple();
    BlockStatus thumb_load_store_immediate();
    BlockStatus thumb_add_sp_pc();
    BlockStatus thumb_adjust_stack_pointer();

    BlockStatus illegal_instruction();

private:
    void emit_advance_pc();
    void emit_link();
    void emit_branch(IRValue address);
    
    u16 code_read_half(u32 addr);
    u32 code_read_word(u32 addr);

    Condition evaluate_arm_condition();
    Condition evaluate_thumb_condition();

    u32 instruction_size{0};
    u32 code_address{0};
    u32 instruction{0};
    Jit& jit;
    IREmitter& ir;
};

} // namespace arm