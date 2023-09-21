#include "common/logger.h"
#include "common/bits.h"
#include "arm/instructions.h"
#include "arm/jit/ir/translator.h"
#include "arm/jit/ir/value.h"
#include "arm/jit/jit.h"

namespace arm {

Translator::BlockStatus Translator::arm_branch_link_maybe_exchange() {
    logger.todo("Translator: handle arm_branch_link_maybe_exchange");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_branch_exchange() {
    logger.todo("Translator: handle arm_branch_exchange");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_count_leading_zeroes() {
    logger.todo("Translator: handle arm_count_leading_zeroes");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_branch_link() {
    logger.todo("Translator: handle arm_branch_link");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_branch_link_exchange() {
    logger.todo("Translator: handle arm_branch_link_exchange");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_branch_link_exchange_register() {
    logger.todo("Translator: handle arm_branch_link_exchange_register");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_single_data_swap() {
    logger.todo("Translator: handle arm_single_data_swap");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_multiply() {
    logger.todo("Translator: handle arm_multiply");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_saturating_add_subtract() {
    logger.todo("Translator: handle arm_saturating_add_subtract");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_multiply_long() {
    logger.todo("Translator: handle arm_multiply_long");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_halfword_data_transfer() {
    logger.todo("Translator: handle arm_halfword_data_transfer");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_status_load() {
    logger.todo("Translator: handle arm_status_load");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_status_store_register() {
    logger.todo("Translator: handle arm_status_store_register");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_status_store_immediate() {
    logger.todo("Translator: handle arm_status_store_immediate");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_block_data_transfer() {
    logger.todo("Translator: handle arm_block_data_transfer");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_single_data_transfer() {
    logger.todo("Translator: handle arm_single_data_transfer");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_data_processing() {
    auto opcode = ARMDataProcessing::decode(instruction);
    auto op1 = ir.load_gpr(opcode.rn);
    IRValue op2;
    auto early_advance_pc = false;

    bool carry_used_in_opcode = opcode.opcode == ARMDataProcessing::Opcode::ADC ||
        opcode.opcode == ARMDataProcessing::Opcode::SBC ||
        opcode.opcode == ARMDataProcessing::Opcode::RSC;

    bool set_carry = opcode.set_flags && !carry_used_in_opcode;

    if (opcode.imm) {
        op2 = ir.constant(opcode.rhs.imm.rotated);

        if (set_carry && opcode.rhs.imm.shift != 0) {
            ir.store_flag(Flag::C, ir.constant(opcode.rhs.imm.rotated >> 31));
        }
    } else {
        logger.todo("handle register");
    }

    logger.todo("Translator: handle arm_data_processing");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_coprocessor_register_transfer() {
    logger.todo("Translator: handle arm_coprocessor_register_transfer");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_software_interrupt() {
    logger.todo("Translator: handle arm_software_interrupt");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_signed_multiply_accumulate_long() {
    logger.todo("Translator: handle arm_signed_multiply_accumulate_long");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_signed_multiply_word() {
    logger.todo("Translator: handle arm_signed_multiply_word");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_signed_multiply() {
    logger.todo("Translator: handle arm_signed_multiply");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_breakpoint() {
    logger.todo("Translator: handle arm_breakpoint");
    return BlockStatus::Continue;
}

} // namespace arm