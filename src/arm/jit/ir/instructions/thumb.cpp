#include "common/logger.h"
#include "common/bits.h"
#include "arm/instructions.h"
#include "arm/jit/ir/translator.h"
#include "arm/jit/ir/types.h"

namespace arm {

Translator::BlockStatus Translator::thumb_alu_immediate(Emitter& emitter) {
    logger.todo("Translator: handle thumb_alu_immediate");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_branch_link_offset(Emitter& emitter) {
    logger.todo("Translator: handle thumb_branch_link_offset");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_branch_link_setup(Emitter& emitter) {
    logger.todo("Translator: handle thumb_branch_link_setup");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_branch_link_exchange_offset(Emitter& emitter) {
    logger.todo("Translator: handle thumb_branch_link_exchange_offset");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_branch(Emitter& emitter) {
    auto opcode = ThumbBranch::decode(instruction);
    auto target_address = code_address + opcode.offset + (2 * instruction_size);
    emitter.store_gpr(GPR::PC, IRConstant{target_address});
    return BlockStatus::Break;
}

Translator::BlockStatus Translator::thumb_push_pop(Emitter& emitter) {
    logger.todo("Translator: handle thumb_push_pop");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_data_processing_register(Emitter& emitter) {
    logger.todo("Translator: handle thumb_data_processing_register");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_special_data_processing(Emitter& emitter) {
    logger.todo("Translator: handle thumb_special_data_processing");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_branch_link_exchange(Emitter& emitter) {
    logger.todo("Translator: handle thumb_branch_link_exchange");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_branch_exchange(Emitter& emitter) {
    logger.todo("Translator: handle thumb_branch_exchange");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_load_store_register_offset(Emitter& emitter) {
    logger.todo("Translator: handle thumb_load_store_register_offset");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_load_store_signed(Emitter& emitter) {
    logger.todo("Translator: handle thumb_load_store_signed");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_load_pc(Emitter& emitter) {
    logger.todo("Translator: handle thumb_load_pc");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_load_store_sp_relative(Emitter& emitter) {
    logger.todo("Translator: handle thumb_load_store_sp_relative");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_load_store_halfword(Emitter& emitter) {
    logger.todo("Translator: handle thumb_load_store_halfword");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_add_subtract(Emitter& emitter) {
    logger.todo("Translator: handle thumb_add_subtract");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_shift_immediate(Emitter& emitter) {
    logger.todo("Translator: handle thumb_shift_immediate");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_software_interrupt(Emitter& emitter) {
    logger.todo("Translator: handle thumb_software_interrupt");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_branch_conditional(Emitter& emitter) {
    auto opcode = ThumbBranchConditional::decode(instruction);
    auto target_address = code_address + opcode.offset + (2 * instruction_size);
    emitter.store_gpr(GPR::PC, IRConstant{target_address});
    return BlockStatus::Break;
}

Translator::BlockStatus Translator::thumb_load_store_multiple(Emitter& emitter) {
    logger.todo("Translator: handle thumb_load_store_multiple");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_load_store_immediate(Emitter& emitter) {
    logger.todo("Translator: handle thumb_load_store_immediate");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_add_sp_pc(Emitter& emitter) {
    logger.todo("Translator: handle thumb_add_sp_pc");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_adjust_stack_pointer(Emitter& emitter) {
    logger.todo("Translator: handle thumb_adjust_stack_pointer");
    return BlockStatus::Continue;
}

} // namespace arm