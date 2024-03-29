#pragma once

#include <array>
#include "arm/cpu.h"
#include "arm/arch.h"
#include "arm/memory.h"
#include "arm/coprocessor.h"
#include "arm/decoder.h"
#include "arm/instructions.h"

namespace arm {

class Interpreter : public CPU {
public:
    Interpreter(Arch arch, Memory& memory, Coprocessor& coprocessor);

    void reset() override;
    void run(int cycles) override;
    void update_irq(bool irq) override;
    bool is_halted() override;
    void update_halted(bool halted) override;
    Arch get_arch() override;

    u32 get_gpr(GPR gpr) override;
    u32 get_gpr(GPR gpr, Mode mode) override;
    void set_gpr(GPR gpr, u32 value) override;
    void set_gpr(GPR gpr, Mode mode, u32 value) override;

    StatusRegister get_cpsr() override;
    void set_cpsr(StatusRegister value) override;
    StatusRegister get_spsr(Mode mode) override;
    void set_spsr(Mode mode, StatusRegister value) override;

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
    void flush_pipeline();
    void arm_flush_pipeline();
    void thumb_flush_pipeline();

    void generate_condition_table();
    bool evaluate_condition(Condition condition);

    void switch_mode(Mode mode);
    Bank get_bank_from_mode(Mode mode);

    void set_nz(u32 result);

    // alu helpers
    std::tuple<u32, std::optional<bool>> barrel_shifter(u32 value, ShiftType shift_type, int amount, bool imm);
    u32 alu_mov(u32 op2, bool set_flags);
    u32 alu_mvn(u32 op2, bool set_flags);
    void alu_teq(u32 op1, u32 op2);
    void alu_cmp(u32 op1, u32 op2);
    void alu_cmn(u32 op1, u32 op2);
    void alu_tst(u32 op1, u32 op2);
    u32 alu_add(u32 op1, u32 op2, bool set_flags);
    u32 alu_adc(u32 op1, u32 op2, bool set_flags);
    u32 alu_sbc(u32 op1, u32 op2, bool set_flags);
    u32 alu_eor(u32 op1, u32 op2, bool set_flags);
    u32 alu_sub(u32 op1, u32 op2, bool set_flags);
    u32 alu_orr(u32 op1, u32 op2, bool set_flags);
    u32 alu_bic(u32 op1, u32 op2, bool set_flags);
    u32 alu_and(u32 op1, u32 op2, bool set_flags);
    u32 alu_lsl(u32 value, int amount, bool& carry);
    u32 alu_lsr(u32 value, int amount, bool& carry, bool imm);
    u32 alu_asr(u32 value, int amount, bool& carry, bool imm);
    u32 alu_ror(u32 value, int amount, bool& carry, bool imm);

    u16 code_read_half(u32 addr);
    u32 code_read_word(u32 addr);
    u8 read_byte(u32 addr);
    u16 read_half(u32 addr);
    u32 read_half_rotate(u32 addr);
    u32 read_word(u32 addr);
    u32 read_word_rotate(u32 addr);
    
    void write_byte(u32 addr, u8 data);
    void write_half(u32 addr, u16 data);
    void write_word(u32 addr, u32 data);

    void handle_interrupt();
    void undefined_exception();
    
    Arch arch;
    Memory& memory;
    Coprocessor& coprocessor;
    std::array<u32, 2> pipeline;
    u32 instruction;
    std::array<std::array<bool, 16>, 16> condition_table;
    bool irq;
    bool halted;
};

} // namespace arm