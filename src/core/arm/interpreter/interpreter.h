#pragma once

#include <array>
#include "core/arm/cpu.h"
#include "core/arm/arch.h"
#include "core/arm/memory.h"
#include "core/arm/coprocessor.h"
#include "core/arm/decoder.h"

namespace core::arm {

class Interpreter : public CPU {
public:
    Interpreter(Arch arch, Memory& memory, Coprocessor& coprocessor);

    void reset() override;
    void run(int cycles) override;
    void jump_to(u32 addr) override;
    void set_mode(Mode mode) override;

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
    void arm_flush_pipeline();
    void thumb_flush_pipeline();

    Arch arch;
    Memory& memory;
    Coprocessor& coprocessor;
    std::array<u32, 2> pipeline;
    u32 instruction;
};

} // namespace core::arm