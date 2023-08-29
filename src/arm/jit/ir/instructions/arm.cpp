#include "common/logger.h"
#include "arm/instructions.h"
#include "arm/jit/ir/translator.h"

namespace arm {

void Translator::arm_branch_link_maybe_exchange() {
    logger.todo("Translator: handle arm_branch_link_maybe_exchange");
}

void Translator::arm_branch_exchange() {
    logger.todo("Translator: handle arm_branch_exchange");
}

void Translator::arm_count_leading_zeroes() {
    logger.todo("Translator: handle arm_count_leading_zeroes");
}

void Translator::arm_branch_link() {
    logger.todo("Translator: handle arm_branch_link");
}

void Translator::arm_branch_link_exchange() {
    logger.todo("Translator: handle arm_branch_link_exchange");
}

void Translator::arm_branch_link_exchange_register() {
    logger.todo("Translator: handle arm_branch_link_exchange_register");
}

void Translator::arm_single_data_swap() {
    logger.todo("Translator: handle arm_single_data_swap");
}

void Translator::arm_multiply() {
    logger.todo("Translator: handle arm_multiply");
}

void Translator::arm_saturating_add_subtract() {
    logger.todo("Translator: handle arm_saturating_add_subtract");
}

void Translator::arm_multiply_long() {
    logger.todo("Translator: handle arm_multiply_long");
}

void Translator::arm_halfword_data_transfer() {
    logger.todo("Translator: handle arm_halfword_data_transfer");
}

void Translator::arm_status_load() {
    logger.todo("Translator: handle arm_status_load");
}

void Translator::arm_status_store_register() {
    logger.todo("Translator: handle arm_status_store_register");
}

void Translator::arm_status_store_immediate() {
    logger.todo("Translator: handle arm_status_store_immediate");
}

void Translator::arm_block_data_transfer() {
    logger.todo("Translator: handle arm_block_data_transfer");
}

void Translator::arm_single_data_transfer() {
    logger.todo("Translator: handle arm_single_data_transfer");
}

void Translator::arm_data_processing() {
    auto opcode = ARMDataProcessing::decode(instruction);

    // TODO: do early pc increment and carry set/clear
    // TODO: do barrel shifter

    // TODO: do different data processing ops translation

    // TODO: check for pc writes

    logger.todo("Translator: handle arm_data_processing");
}

void Translator::arm_coprocessor_register_transfer() {
    logger.todo("Translator: handle arm_coprocessor_register_transfer");
}

void Translator::arm_software_interrupt() {
    logger.todo("Translator: handle arm_software_interrupt");
}

void Translator::arm_signed_multiply_accumulate_long() {
    logger.todo("Translator: handle arm_signed_multiply_accumulate_long");
}

void Translator::arm_signed_multiply_word() {
    logger.todo("Translator: handle arm_signed_multiply_word");
}

void Translator::arm_signed_multiply() {
    logger.todo("Translator: handle arm_signed_multiply");
}

void Translator::arm_breakpoint() {
    logger.todo("Translator: handle arm_breakpoint");
}


} // namespace arm