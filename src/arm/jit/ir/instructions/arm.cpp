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
    auto opcode = ARMSingleDataTransfer::decode(instruction);
    IRValue op2;
    IRVariable addr = emitter.load_gpr(opcode.rn);
    bool do_writeback = !opcode.load || opcode.rd != opcode.rn;

    if (opcode.imm) {
        op2 = IRConstant{opcode.rhs.imm};
    } else {
        logger.todo("Translator: arm_single_data_transfer handle barrel shifter");
    }

    if (!opcode.up) {
        logger.todo("Translator: arm_single_data_transfer handle down");
    }

    if (opcode.pre) {
        addr = emitter.add(addr, op2, false);
    }

    emit_advance_pc(emitter);

    if (opcode.load) {
        if (opcode.byte) {
            logger.todo("Translator: handle load byte");
        } else {
            logger.todo("Translator: handle read word and rotate");
        }
    } else {
        IRVariable src = emitter.load_gpr(opcode.rd);
        if (opcode.byte) {
            logger.todo("Translator: handle write byte");
        } else {
            emitter.memory_write(addr, src, AccessType::Word);
        }
    }

    if (do_writeback) {
        if (!opcode.pre) {
            auto base = emitter.load_gpr(opcode.rn);
            auto new_base = emitter.add(base, op2, false);
            emitter.store_gpr(opcode.rn, new_base);
        } else if (opcode.writeback) {
            emitter.store_gpr(opcode.rn, addr);
        }
    }

    if (opcode.load && opcode.rd == 15) {
        logger.todo("Translator: handle arm_single_data_transfer pc write");
    }
}

void Translator::arm_data_processing(Emitter& emitter) {
    auto opcode = ARMDataProcessing::decode(instruction);

    // TODO: only some opcodes rely on the barrel shifter to update the carry flag,
    // so later only emit code to set/clear the carry flag if necessary.
    // right now the carry flag would just get overwritten by opcodes like sub, rsb, etc
    bool carry_done_in_opcode = opcode.opcode == ARMDataProcessing::Opcode::ADC ||
        opcode.opcode == ARMDataProcessing::Opcode::SBC ||
        opcode.opcode == ARMDataProcessing::Opcode::RSC;

    bool set_carry = opcode.set_flags && !carry_done_in_opcode;
    bool early_advance_pc = false;
    IRValue op2;

    if (opcode.imm) {
        op2 = IRConstant{opcode.rhs.imm.rotated};

        if (set_carry && opcode.rhs.imm.shift != 0) {
            if (common::get_bit<31>(opcode.rhs.imm.rotated)) {
                emitter.set_carry();
            } else {
                emitter.clear_carry();
            }
        }
    } else {
        IRValue amount;
        op2 = emitter.load_gpr(opcode.rhs.reg.rm);

        if (opcode.rhs.reg.imm) {
            amount = IRConstant{opcode.rhs.reg.amount.imm};
        } else {
            // TODO: do early pc increment
            logger.todo("Translator: handle arm data processing shift by register");
        }

        switch (opcode.rhs.reg.shift_type) {
        case ShiftType::LSL:
            op2 = emitter.logical_shift_left(op2, amount, set_carry);
            break;
        case ShiftType::LSR:
            op2 = emitter.logical_shift_right(op2, amount, set_carry);
            break;
        case ShiftType::ASR:
            logger.todo("Translator: handle asr");
            break;
        case ShiftType::ROR:
            logger.todo("Translator: handle ror");
            break;
        }
    }

    switch (opcode.opcode) {
    case ARMDataProcessing::Opcode::AND: {
        IRValue op1 = emitter.load_gpr(opcode.rn);
        auto dst = emitter._and(op1, op2, opcode.set_flags);
        emitter.store_gpr(opcode.rd, dst);
        break;
    }
    case ARMDataProcessing::Opcode::MOV: {
        auto dst = emitter.move(op2, opcode.set_flags);
        emitter.store_gpr(opcode.rd, dst);
        break;
    }
    default:
        logger.todo("Translator: handle data processing opcode %d", static_cast<u8>(opcode.opcode));
    }

    if (opcode.rd == 15) {
        logger.todo("Translator: handle pc write in data processing");
    } else if (!early_advance_pc) {
        emit_advance_pc(emitter);
    }
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