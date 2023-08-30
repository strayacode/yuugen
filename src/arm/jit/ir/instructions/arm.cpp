#include "common/logger.h"
#include "common/bits.h"
#include "arm/instructions.h"
#include "arm/jit/ir/translator.h"
#include "arm/jit/ir/types.h"

namespace arm {

void Translator::arm_branch_link_maybe_exchange(Emitter& emitter) {
    logger.todo("Translator: handle arm_branch_link_maybe_exchange");
}

void Translator::arm_branch_exchange(Emitter& emitter) {
    logger.todo("Translator: handle arm_branch_exchange");
}

void Translator::arm_count_leading_zeroes(Emitter& emitter) {
    logger.todo("Translator: handle arm_count_leading_zeroes");
}

void Translator::arm_branch_link(Emitter& emitter) {
    logger.todo("Translator: handle arm_branch_link");
}

void Translator::arm_branch_link_exchange(Emitter& emitter) {
    logger.todo("Translator: handle arm_branch_link_exchange");
}

void Translator::arm_branch_link_exchange_register(Emitter& emitter) {
    logger.todo("Translator: handle arm_branch_link_exchange_register");
}

void Translator::arm_single_data_swap(Emitter& emitter) {
    logger.todo("Translator: handle arm_single_data_swap");
}

void Translator::arm_multiply(Emitter& emitter) {
    logger.todo("Translator: handle arm_multiply");
}

void Translator::arm_saturating_add_subtract(Emitter& emitter) {
    logger.todo("Translator: handle arm_saturating_add_subtract");
}

void Translator::arm_multiply_long(Emitter& emitter) {
    logger.todo("Translator: handle arm_multiply_long");
}

void Translator::arm_halfword_data_transfer(Emitter& emitter) {
    logger.todo("Translator: handle arm_halfword_data_transfer");
}

void Translator::arm_status_load(Emitter& emitter) {
    logger.todo("Translator: handle arm_status_load");
}

void Translator::arm_status_store_register(Emitter& emitter) {
    logger.todo("Translator: handle arm_status_store_register");
}

void Translator::arm_status_store_immediate(Emitter& emitter) {
    logger.todo("Translator: handle arm_status_store_immediate");
}

void Translator::arm_block_data_transfer(Emitter& emitter) {
    logger.todo("Translator: handle arm_block_data_transfer");
}

void Translator::arm_single_data_transfer(Emitter& emitter) {
    logger.todo("Translator: handle arm_single_data_transfer");
}

void Translator::arm_data_processing(Emitter& emitter) {
    auto opcode = ARMDataProcessing::decode(instruction);

    // TODO: only some opcodes rely on the barrel shifter to update the carry flag,
    // so later only emit code to set/clear the carry flag if necessary.
    // right now the carry flag would just get overwritten by opcodes like sub, rsb, etc
    bool carry_done_in_opcode = opcode.opcode == ARMDataProcessing::Opcode::ADC ||
        opcode.opcode == ARMDataProcessing::Opcode::SBC ||
        opcode.opcode == ARMDataProcessing::Opcode::RSC;

    bool update_carry = opcode.set_flags && !carry_done_in_opcode;
    bool early_advance_pc = false;
    IRValue op2;

    if (opcode.imm) {
        op2 = IRConstant{opcode.rhs.imm.rotated};

        if (update_carry && opcode.rhs.imm.shift != 0) {
            if (common::get_bit<31>(opcode.rhs.imm.rotated)) {
                emitter.set_carry();
            } else {
                emitter.clear_carry();
            }
        }
    } else {
        logger.todo("Translator: handle register arm data processing");
    }

    // TODO: do early pc increment
    // TODO: do barrel shifter

    switch (opcode.opcode) {
    case ARMDataProcessing::Opcode::MOV:
        emitter.move(op2, opcode.set_flags);
        break;
    default:
        logger.todo("Translator: handle data processing opcode %d", static_cast<u8>(opcode.opcode));
    }

    emitter.basic_block.dump();

    if (opcode.rd == 15) {
        logger.todo("Translator: handle pc write in data processing");
    } else if (!early_advance_pc) {
        logger.todo("Translator: advance pc at end of arm_data_processing");
    }

    // TODO: check for pc writes

    logger.todo("Translator: handle arm_data_processing");
}

void Translator::arm_coprocessor_register_transfer(Emitter& emitter) {
    logger.todo("Translator: handle arm_coprocessor_register_transfer");
}

void Translator::arm_software_interrupt(Emitter& emitter) {
    logger.todo("Translator: handle arm_software_interrupt");
}

void Translator::arm_signed_multiply_accumulate_long(Emitter& emitter) {
    logger.todo("Translator: handle arm_signed_multiply_accumulate_long");
}

void Translator::arm_signed_multiply_word(Emitter& emitter) {
    logger.todo("Translator: handle arm_signed_multiply_word");
}

void Translator::arm_signed_multiply(Emitter& emitter) {
    logger.todo("Translator: handle arm_signed_multiply");
}

void Translator::arm_breakpoint(Emitter& emitter) {
    logger.todo("Translator: handle arm_breakpoint");
}


} // namespace arm