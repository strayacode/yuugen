#include "common/logger.h"
#include "common/bits.h"
#include "arm/instructions.h"
#include "arm/jit/ir/translator.h"
#include "arm/jit/ir/value.h"

namespace arm {

Translator::BlockStatus Translator::thumb_alu_immediate() {
    logger.todo("Translator: handle thumb_alu_immediate");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_branch_link_offset() {
    logger.todo("Translator: handle thumb_branch_link_offset");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_branch_link_setup() {
    logger.todo("Translator: handle thumb_branch_link_setup");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_branch_link_exchange_offset() {
    logger.todo("Translator: handle thumb_branch_link_exchange_offset");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_branch() {
    logger.todo("Translator: handle thumb_branch");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_push_pop() {
    logger.todo("Translator: handle thumb_push_pop");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_data_processing_register() {
    logger.todo("Translator: handle thumb_data_processing_register");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_special_data_processing() {
    logger.todo("Translator: handle thumb_special_data_processing");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_branch_link_exchange() {
    logger.todo("Translator: handle thumb_branch_link_exchange");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_branch_exchange() {
    logger.todo("Translator: handle thumb_branch_exchange");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_load_store_register_offset() {
    logger.todo("Translator: handle thumb_load_store_register_offset");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_load_store_signed() {
    logger.todo("Translator: handle thumb_load_store_signed");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_load_pc() {
    logger.todo("Translator: handle thumb_load_pc");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_load_store_sp_relative() {
    logger.todo("Translator: handle thumb_load_store_sp_relative");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_load_store_halfword() {
    logger.todo("Translator: handle thumb_load_store_halfword");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_add_subtract() {
    logger.todo("Translator: handle thumb_add_subtract");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_shift_immediate() {
    logger.todo("Translator: handle thumb_shift_immediate");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_software_interrupt() {
    logger.todo("Translator: handle thumb_software_interrupt");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_branch_conditional() {
    logger.todo("Translator: handle thumb_branch_conditional");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_load_store_multiple() {
    logger.todo("Translator: handle thumb_load_store_multiple");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_load_store_immediate() {
    logger.todo("Translator: handle thumb_load_store_immediate");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_add_sp_pc() {
    logger.todo("Translator: handle thumb_add_sp_pc");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_adjust_stack_pointer() {
    logger.todo("Translator: handle thumb_adjust_stack_pointer");
    return BlockStatus::Continue;
}

} // namespace arm