#include "common/logger.h"
#include "common/bits.h"
#include "arm/instructions.h"
#include "arm/jit/ir/translator.h"
#include "arm/jit/ir/value.h"

namespace arm {

Translator::BlockStatus Translator::thumb_alu_immediate() {
    auto opcode = ThumbALUImmediate::decode(instruction);
    IRConstant imm{opcode.imm};

    switch (opcode.opcode) {
    case ThumbALUImmediate::Opcode::MOV:
        ir.store_gpr(opcode.rd, imm);
        ir.update_flag(Flags::N, false);
        ir.update_flag(Flags::Z, opcode.imm == 0);
        break;
    case ThumbALUImmediate::Opcode::CMP: {
        auto value = ir.load_gpr(opcode.rd);
        ir.compare(value, imm);
        break;
    }
    case ThumbALUImmediate::Opcode::ADD: {
        auto value = ir.load_gpr(opcode.rd);
        auto result = ir.add(value, imm, true);
        ir.store_gpr(opcode.rd, result);
        break;
    }
    case ThumbALUImmediate::Opcode::SUB: {
        auto value = ir.load_gpr(opcode.rd);
        auto result = ir.sub(value, imm, true);
        ir.store_gpr(opcode.rd, result);
        break;
    }
    }

    emit_advance_pc();
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
    auto opcode = ThumbBranch::decode(instruction);
    auto target_address = code_address + opcode.offset + (2 * instruction_size);
    ir.store_gpr(GPR::PC, IRConstant{target_address});
    return BlockStatus::Break;
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
    auto opcode = ThumbShiftImmediate::decode(instruction);
    auto src = ir.load_gpr(opcode.rs);
    auto value = emit_barrel_shifter(src, opcode.shift_type, IRConstant{opcode.amount}, true);
    ir.store_flags(Flags::C);

    // TODO: does this need to be copied
    auto dst = ir.copy(value);
    ir.store_gpr(opcode.rd, dst);
    ir.store_flags(Flags::NZ);

    emit_advance_pc();
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_software_interrupt() {
    logger.todo("Translator: handle thumb_software_interrupt");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_branch_conditional() {
    auto opcode = ThumbBranchConditional::decode(instruction);
    auto target_address = code_address + opcode.offset + (2 * instruction_size);
    ir.store_gpr(GPR::PC, IRConstant{target_address});
    return BlockStatus::Break;
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