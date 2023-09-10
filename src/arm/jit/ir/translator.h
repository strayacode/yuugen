#pragma once

#include "arm/cpu.h"
#include "arm/instructions.h"
#include "arm/jit/basic_block.h"
#include "arm/jit/ir/emitter.h"

namespace arm {

class Jit;

class Translator {
public:
    Translator(Arch arch, Jit& jit);

    void translate(BasicBlock& basic_block);

    enum class BlockStatus {
        Break,
        Continue,
    };

    // arm instruction handlers
    BlockStatus arm_branch_link_maybe_exchange(Emitter& emitter);
    BlockStatus arm_branch_exchange(Emitter& emitter);
    BlockStatus arm_count_leading_zeroes(Emitter& emitter);
    BlockStatus arm_branch_link(Emitter& emitter);
    BlockStatus arm_branch_link_exchange(Emitter& emitter);
    BlockStatus arm_branch_link_exchange_register(Emitter& emitter);
    BlockStatus arm_single_data_swap(Emitter& emitter);
    BlockStatus arm_multiply(Emitter& emitter);
    BlockStatus arm_saturating_add_subtract(Emitter& emitter);
    BlockStatus arm_multiply_long(Emitter& emitter);
    BlockStatus arm_halfword_data_transfer(Emitter& emitter);
    BlockStatus arm_status_load(Emitter& emitter);
    BlockStatus arm_status_store_register(Emitter& emitter);
    BlockStatus arm_status_store_immediate(Emitter& emitter);
    BlockStatus arm_block_data_transfer(Emitter& emitter);
    BlockStatus arm_single_data_transfer(Emitter& emitter);
    BlockStatus arm_data_processing(Emitter& emitter);
    BlockStatus arm_coprocessor_register_transfer(Emitter& emitter);
    BlockStatus arm_software_interrupt(Emitter& emitter);
    BlockStatus arm_signed_multiply_accumulate_long(Emitter& emitter);
    BlockStatus arm_signed_multiply_word(Emitter& emitter);
    BlockStatus arm_signed_multiply(Emitter& emitter);
    BlockStatus arm_breakpoint(Emitter& emitter);

    // thumb instruction handlers
    BlockStatus thumb_alu_immediate(Emitter& emitter);
    BlockStatus thumb_branch_link_offset(Emitter& emitter);
    BlockStatus thumb_branch_link_setup(Emitter& emitter);
    BlockStatus thumb_branch_link_exchange_offset(Emitter& emitter);
    BlockStatus thumb_branch(Emitter& emitter);
    BlockStatus thumb_push_pop(Emitter& emitter);
    BlockStatus thumb_data_processing_register(Emitter& emitter);
    BlockStatus thumb_special_data_processing(Emitter& emitter);
    BlockStatus thumb_branch_link_exchange(Emitter& emitter);
    BlockStatus thumb_branch_exchange(Emitter& emitter);
    BlockStatus thumb_load_store_register_offset(Emitter& emitter);
    BlockStatus thumb_load_store_signed(Emitter& emitter);
    BlockStatus thumb_load_pc(Emitter& emitter);
    BlockStatus thumb_load_store_sp_relative(Emitter& emitter);
    BlockStatus thumb_load_store_halfword(Emitter& emitter);
    BlockStatus thumb_add_subtract(Emitter& emitter);
    BlockStatus thumb_shift_immediate(Emitter& emitter);
    BlockStatus thumb_software_interrupt(Emitter& emitter);
    BlockStatus thumb_branch_conditional(Emitter& emitter);
    BlockStatus thumb_load_store_multiple(Emitter& emitter);
    BlockStatus thumb_load_store_immediate(Emitter& emitter);
    BlockStatus thumb_add_sp_pc(Emitter& emitter);
    BlockStatus thumb_adjust_stack_pointer(Emitter& emitter);

    BlockStatus illegal_instruction(Emitter& emitter);

private:
    void emit_advance_pc(Emitter& emitter);
    void emit_link(Emitter& emitter);
    void emit_set_carry(Emitter& emitter);
    IRVariable emit_barrel_shifter(Emitter& emitter, IRValue value, ShiftType shift_type, IRValue amount, bool set_carry);

    u16 code_read_half(u32 addr);
    u32 code_read_word(u32 addr);

    Condition evaluate_arm_condition();
    Condition evaluate_thumb_condition();

    u32 instruction_size{0};
    u32 code_address{0};
    u32 instruction{0};
    Arch arch;
    Jit& jit;
};

} // namespace arm