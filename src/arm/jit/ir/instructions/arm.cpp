#include "common/logger.h"
#include "common/bits.h"
#include "arm/instructions.h"
#include "arm/jit/ir/translator.h"
#include "arm/jit/ir/types.h"

namespace arm {

Translator::BlockStatus Translator::arm_branch_link_maybe_exchange(Emitter& emitter) {
    if (emitter.basic_block.condition != Condition::NV) {
        return arm_branch_link(emitter);
    } else {
        return arm_branch_link_exchange(emitter);
    }
}

Translator::BlockStatus Translator::arm_branch_exchange(Emitter& emitter) {
    logger.todo("Translator: handle arm_branch_exchange");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_count_leading_zeroes(Emitter& emitter) {
    logger.todo("Translator: handle arm_count_leading_zeroes");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_branch_link(Emitter& emitter) {
    auto opcode = ARMBranchLink::decode(instruction);
    if (opcode.link) {
        emit_link(emitter);
    }

    auto target_address = code_address + opcode.offset + (2 * instruction_size);
    emitter.store_gpr(GPR::PC, IRConstant{target_address});
    return BlockStatus::Break;
}

Translator::BlockStatus Translator::arm_branch_link_exchange(Emitter& emitter) {
    logger.todo("Translator: handle arm_branch_link_exchange");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_branch_link_exchange_register(Emitter& emitter) {
    logger.todo("Translator: handle arm_branch_link_exchange_register");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_single_data_swap(Emitter& emitter) {
    logger.todo("Translator: handle arm_single_data_swap");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_multiply(Emitter& emitter) {
    logger.todo("Translator: handle arm_multiply");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_saturating_add_subtract(Emitter& emitter) {
    logger.todo("Translator: handle arm_saturating_add_subtract");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_multiply_long(Emitter& emitter) {
    logger.todo("Translator: handle arm_multiply_long");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_halfword_data_transfer(Emitter& emitter) {
    auto opcode = ARMHalfwordDataTransfer::decode(instruction);

    if (opcode.rd == 15) {
        logger.todo("Translator: handle rd == 15 in arm_halfword_data_transfer");
    }

    IRValue op2;
    IRVariable addr = emitter.load_gpr(opcode.rn);
    bool do_writeback = !opcode.load || opcode.rd != opcode.rn;

    if (opcode.imm) {
        op2 = IRConstant{opcode.rhs.imm};
    } else {
        op2 = emitter.load_gpr(opcode.rhs.rm);
    }

    if (!opcode.up) {
        logger.todo("Translator: arm_halfword_data_transfer handle down");
    }

    if (opcode.pre) {
        addr = emitter.add(addr, op2, false);
    }

    emit_advance_pc(emitter);

    if (opcode.half && opcode.sign) {
        if (opcode.load) {
            logger.todo("Translator: handle ldrsh");
        } else if (arch == Arch::ARMv5) {

            logger.todo("Translator: handle strd");
        }
    } else if (opcode.half) {
        if (opcode.load) {
            logger.todo("Translator: handle ldrh");
        } else {
            IRVariable src = emitter.load_gpr(opcode.rd);
            emitter.memory_write(addr, src, AccessType::Half);
        }
    } else if (opcode.sign) {
        if (opcode.load) {
            logger.todo("Translator: handle ldrsb");
        } else if (arch == Arch::ARMv5) {
            logger.todo("Translator: handle ldrd");
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

    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_status_load(Emitter& emitter) {
    logger.todo("Translator: handle arm_status_load");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_status_store_register(Emitter& emitter) {
    logger.todo("Translator: handle arm_status_store_register");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_status_store_immediate(Emitter& emitter) {
    logger.todo("Translator: handle arm_status_store_immediate");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_block_data_transfer(Emitter& emitter) {
    logger.todo("Translator: handle arm_block_data_transfer");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_single_data_transfer(Emitter& emitter) {
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

    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_data_processing(Emitter& emitter) {
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
    case ARMDataProcessing::Opcode::EOR:
        logger.todo("Translator: handle sub");
        break;
    case ARMDataProcessing::Opcode::SUB: {
        IRValue op1 = emitter.load_gpr(opcode.rn);
        auto dst = emitter.sub(op1, op2, opcode.set_flags);
        emitter.store_gpr(opcode.rd, dst);
        break;
    }
    case ARMDataProcessing::Opcode::RSB:
       logger.todo("Translator: handle rsb");
        break;
    case ARMDataProcessing::Opcode::ADD:
        logger.todo("Translator: handle add");
        break;
    case ARMDataProcessing::Opcode::ADC:
        logger.todo("Translator: handle adc");
        break;
    case ARMDataProcessing::Opcode::SBC:
        logger.todo("Translator: handle sbc");
        break;
    case ARMDataProcessing::Opcode::RSC:
        logger.todo("Translator: handle rsc");
        break;
    case ARMDataProcessing::Opcode::TST:
        logger.todo("Translator: handle tst");
        break;
    case ARMDataProcessing::Opcode::TEQ:
        logger.todo("Translator: handle teq");
        break;
    case ARMDataProcessing::Opcode::CMP:
        logger.todo("Translator: handle cmp");
        break;
    case ARMDataProcessing::Opcode::CMN:
        logger.todo("Translator: handle cmn");
        break;
    case ARMDataProcessing::Opcode::ORR:
        logger.todo("Translator: handle orr");
        break;
    case ARMDataProcessing::Opcode::MOV: {
        auto dst = emitter.move(op2, opcode.set_flags);
        emitter.store_gpr(opcode.rd, dst);
        break;
    }
    case ARMDataProcessing::Opcode::BIC:
        logger.todo("Translator: handle bic");
        break;
    case ARMDataProcessing::Opcode::MVN:
        logger.todo("Translator: handle mvn");
        break;
    }

    if (opcode.set_flags) {
        std::array<Flags, 16> flags{
            Flags::NZ, Flags::NZ, Flags::NZCV, Flags::NZCV,
            Flags::NZCV, Flags::NZCV, Flags::NZCV, Flags::NZCV,
            Flags::NZ, Flags::NZ, Flags::NZCV, Flags::NZCV,
            Flags::NZ, Flags::NZ, Flags::NZ, Flags::NZ
        };

        emitter.store_flags(flags[static_cast<int>(opcode.opcode)]);
    }

    if (opcode.rd == 15) {
        logger.todo("Translator: handle pc write in data processing");
    } else if (!early_advance_pc) {
        emit_advance_pc(emitter);
    }

    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_coprocessor_register_transfer(Emitter& emitter) {
    logger.todo("Translator: handle arm_coprocessor_register_transfer");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_software_interrupt(Emitter& emitter) {
    logger.todo("Translator: handle arm_software_interrupt");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_signed_multiply_accumulate_long(Emitter& emitter) {
    logger.todo("Translator: handle arm_signed_multiply_accumulate_long");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_signed_multiply_word(Emitter& emitter) {
    logger.todo("Translator: handle arm_signed_multiply_word");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_signed_multiply(Emitter& emitter) {
    logger.todo("Translator: handle arm_signed_multiply");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_breakpoint(Emitter& emitter) {
    logger.todo("Translator: handle arm_breakpoint");
    return BlockStatus::Continue;
}


} // namespace arm