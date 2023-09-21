#include "common/logger.h"
#include "common/bits.h"
#include "arm/instructions.h"
#include "arm/jit/ir/translator.h"
#include "arm/jit/ir/value.h"
#include "arm/jit/jit.h"

namespace arm {

Translator::BlockStatus Translator::arm_branch_link_maybe_exchange() {
    if (ir.basic_block.condition != Condition::NV) {
        return arm_branch_link();
    } else {
        return arm_branch_link_exchange();
    }
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
    auto opcode = ARMBranchLink::decode(instruction);
    if (opcode.link) {
        ir.link();
    }

    ir.branch(ir.constant(ir.basic_block.current_address + opcode.offset));
    return BlockStatus::Break;
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
    IRValue op2;
    auto early_advance_pc = false;

    bool is_comparison_opcode = opcode.opcode == ARMDataProcessing::Opcode::TST ||
        opcode.opcode == ARMDataProcessing::Opcode::TEQ ||
        opcode.opcode == ARMDataProcessing::Opcode::CMP ||
        opcode.opcode == ARMDataProcessing::Opcode::CMN;

    bool carry_used_in_opcode = opcode.opcode == ARMDataProcessing::Opcode::ADC ||
        opcode.opcode == ARMDataProcessing::Opcode::SBC ||
        opcode.opcode == ARMDataProcessing::Opcode::RSC;

    bool uses_op1 = opcode.opcode != ARMDataProcessing::Opcode::MOV &&
        opcode.opcode != ARMDataProcessing::Opcode::MVN;

    bool set_carry = opcode.set_flags && !carry_used_in_opcode;
    bool update_flags = opcode.set_flags && (opcode.rd != GPR::PC || is_comparison_opcode);

    if (opcode.imm) {
        op2 = ir.constant(opcode.rhs.imm.rotated);

        if (set_carry && opcode.rhs.imm.shift != 0) {
            ir.store_flag(Flag::C, ir.constant(opcode.rhs.imm.rotated >> 31));
        }
    } else {
        logger.todo("handle register");
    }

    IRValue op1;
    IRVariable result;

    if (uses_op1) {
        op1 = ir.load_gpr(opcode.rn);
    }

    switch (opcode.opcode) {
    case ARMDataProcessing::Opcode::AND:
        logger.todo("handle and");
        break;
    case ARMDataProcessing::Opcode::EOR:
        logger.todo("handle eor");
        break;
    case ARMDataProcessing::Opcode::SUB:
        logger.todo("handle sub");
        break;
    case ARMDataProcessing::Opcode::RSB:
        logger.todo("handle rsb");
        break;
    case ARMDataProcessing::Opcode::ADD:
        result = ir.add(op1, op2);
        if (update_flags) {
            ir.store_nz(result);
            ir.store_add_cv(op1, op2, result);
        }
        
        break;
    case ARMDataProcessing::Opcode::ADC:
        logger.todo("handle adc");
        break;
    case ARMDataProcessing::Opcode::SBC:
        logger.todo("handle sbc");
        break;
    case ARMDataProcessing::Opcode::RSC:
        logger.todo("handle rsc");
        break;
    case ARMDataProcessing::Opcode::TST:
        logger.todo("handle tst");
        break;
    case ARMDataProcessing::Opcode::TEQ:
        logger.todo("handle teq");
        break;
    case ARMDataProcessing::Opcode::CMP:
        logger.todo("handle cmp");
        break;
    case ARMDataProcessing::Opcode::CMN:
        logger.todo("handle cmn");
        break;
    case ARMDataProcessing::Opcode::ORR:
        logger.todo("handle orr");
        break;
    case ARMDataProcessing::Opcode::MOV:
        result = ir.copy(op2);
        if (update_flags) {
            ir.store_nz(result);
        }

        break;
    case ARMDataProcessing::Opcode::BIC:
        logger.todo("handle bic");
        break;
    case ARMDataProcessing::Opcode::MVN:
        logger.todo("handle mvn");
        break;
    }

    if (result.is_assigned()) {
        ir.store_gpr(opcode.rd, result);
    }

    if (opcode.rd == 15 && opcode.set_flags) {
        ir.copy_spsr_to_cpsr();
    }

    if (opcode.rd == 15 && !is_comparison_opcode) {
        if (opcode.set_flags) {
            logger.todo("Translator: handle pc write in data processing with set flags (t bit might change)");
        }

        return BlockStatus::Break;
    } else if (!early_advance_pc) {
        ir.advance_pc();
    }

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