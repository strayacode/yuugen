#pragma once

#include "Common/Types.h"
#include "Core/ARM/CPUBase.h"

class System;

class Interpreter final : public CPUBase {
public:
    Interpreter(MemoryBase& memory, CoprocessorBase& coprocessor, Arch arch);

    bool run(u64 target) override;
    u64 single_step();

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
    void arm_psr_transfer();
    void arm_block_data_transfer();
    void arm_single_data_transfer();
    void arm_data_processing();
    void arm_coprocessor_register_transfer();
    void arm_software_interrupt();
    void arm_signed_halfword_accumulate_long();
    void arm_signed_halfword_word_multiply();
    void arm_signed_halfword_multiply();
    void arm_breakpoint();

    // arm helpers
    u32 arm_get_shifted_register_data_processing(u32 op2, u8 shift_type, u8 shift_amount, u8& carry_flag, bool shift_imm);
    u32 arm_get_shifted_register_single_data_transfer();

    // alu helpers
    u32 alu_mov(u32 op2, u8 set_flags);
    u32 alu_mvn(u32 op2, u8 set_flags);
    void alu_teq(u32 op1, u32 op2);
    void alu_cmp(u32 op1, u32 op2);
    void alu_cmn(u32 op1, u32 op2);
    void alu_tst(u32 op1, u32 op2);
    u32 alu_add(u32 op1, u32 op2, u8 set_flags);
    u32 alu_adc(u32 op1, u32 op2, u8 set_flags);
    u32 alu_sbc(u32 op1, u32 op2, u8 set_flags);
    u32 alu_eor(u32 op1, u32 op2, u8 set_flags);
    u32 alu_sub(u32 op1, u32 op2, u8 set_flags);
    u32 alu_orr(u32 op1, u32 op2, u8 set_flags);
    u32 alu_bic(u32 op1, u32 op2, u8 set_flags);
    u32 alu_and(u32 op1, u32 op2, u8 set_flags);
    u32 alu_lsl(u32 op1, u8 shift_amount, u8& carry_flag);
    u32 alu_lsr(u32 op1, u8 shift_amount, u8& carry_flag, bool shift_imm);
    u32 alu_asr(u32 op1, u8 shift_amount, u8& carry_flag, bool shift_imm);
    u32 alu_ror(u32 op1, u8 shift_amount, u8& carry_flag, bool shift_imm);

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
    void thumb_load_store();
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

    void unknown_instruction();

private:
    using Instruction = void (Interpreter::*)();

    u8 read_byte(u32 addr);
    u16 read_half(u32 addr);
    u32 read_word(u32 addr);
    u32 read_word_rotate(u32 addr);

    void write_byte(u32 addr, u8 data);
    void write_half(u32 addr, u16 data);
    void write_word(u32 addr, u32 data);

    void arm_flush_pipeline() override;
    void thumb_flush_pipeline() override;

    void handle_interrupt();

    void generate_condition_table();
    bool evaluate_condition(u32 condition);

    void log_state();

    // pipeline state
    std::array<u32, 2> m_pipeline;

    // the currently executing instruction
    u32 m_instruction;

    // condition lut to precompute all condition evaluations
    std::array<std::array<bool, 16>, 16> m_condition_table;
};

