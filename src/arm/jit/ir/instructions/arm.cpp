#include "common/logger.h"
#include "common/bits.h"
#include "arm/instructions.h"
#include "arm/jit/ir/translator.h"
#include "arm/jit/ir/types.h"
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
    auto opcode = ARMBranchExchange::decode(instruction);
    auto address = ir.load_gpr(opcode.rm);
    ir.branch_exchange(address, ExchangeType::Bit0);
    return BlockStatus::Break;
}

Translator::BlockStatus Translator::arm_count_leading_zeroes() {
    logger.todo("Translator: handle arm_count_leading_zeroes");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_branch_link() {
    auto opcode = ARMBranchLink::decode(instruction);
    if (opcode.link) {
        emit_link();
    }

    emit_branch(IRConstant{code_address + opcode.offset});
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
    auto opcode = ARMHalfwordDataTransfer::decode(instruction);

    if (opcode.rd == 15) {
        logger.todo("Translator: handle rd == 15 in arm_halfword_data_transfer");
    }

    IRValue op2;
    IRVariable address = ir.load_gpr(opcode.rn);
    bool do_writeback = !opcode.load || opcode.rd != opcode.rn;

    if (opcode.imm) {
        op2 = IRConstant{opcode.rhs.imm};
    } else {
        op2 = ir.load_gpr(opcode.rhs.rm);
    }

    if (!opcode.up) {
        logger.todo("Translator: arm_halfword_data_transfer handle down");
    }

    if (opcode.pre) {
        address = ir.add(address, op2, false);
    }

    emit_advance_pc();

    if (opcode.half && opcode.sign) {
        if (opcode.load) {
            logger.todo("Translator: handle ldrsh");
        } else if (jit.arch == Arch::ARMv5) {

            logger.todo("Translator: handle strd");
        }
    } else if (opcode.half) {
        if (opcode.load) {
            auto dst = ir.memory_read(address, AccessSize::Half, AccessType::Aligned);
            ir.store_gpr(opcode.rd, dst);
        } else {
            IRVariable src = ir.load_gpr(opcode.rd);
            ir.memory_write(address, src, AccessSize::Half, AccessType::Aligned);
        }
    } else if (opcode.sign) {
        if (opcode.load) {
            logger.todo("Translator: handle ldrsb");
        } else if (jit.arch == Arch::ARMv5) {
            logger.todo("Translator: handle ldrd");
        }
    }

    if (do_writeback) {
        if (!opcode.pre) {
            auto base = ir.load_gpr(opcode.rn);
            auto new_base = ir.add(base, op2, false);
            ir.store_gpr(opcode.rn, new_base);
        } else if (opcode.writeback) {
            ir.store_gpr(opcode.rn, address);
        }
    }

    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_status_load() {
    logger.todo("Translator: handle arm_status_load");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_status_store_register() {
    auto opcode = ARMStatusStore::decode(instruction);
    auto value = ir.load_gpr(opcode.rhs.rm);
    auto value_masked = ir.and_(value, IRConstant{opcode.mask}, false);
    IRVariable psr;

    if (opcode.spsr) {
        psr = ir.load_spsr();
    } else {
        psr = ir.load_cpsr();
    }

    auto psr_masked = ir.and_(psr, IRConstant{~opcode.mask}, false);
    auto psr_new = ir.or_(psr_masked, value_masked, false);

    emit_advance_pc();

    if (opcode.spsr) {
        logger.todo("Translator: modify spsr");
    } else {
        ir.store_cpsr(psr_new);
        
        if (opcode.mask & 0xff) {
            return BlockStatus::Break;
        }
    }

    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_status_store_immediate() {
    logger.todo("Translator: handle arm_status_store_immediate");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_block_data_transfer() {
    auto opcode = ARMBlockDataTransfer::decode(instruction);
    auto address = ir.load_gpr(opcode.rn);
    auto first = 0;
    u32 bytes = 0;
    auto old_mode = ir.basic_block.location.get_mode();
    IRValue new_base;

    if (opcode.rlist != 0) {
        for (int i = 15; i >= 0; i--) {
            if (opcode.rlist & (1 << i)) {
                first = i;
                bytes += 4;
            }
        }
    } else {
        bytes = 0x40;
        if (jit.arch == Arch::ARMv4) {
            opcode.rlist = 1 << 15;
            opcode.r15_in_rlist = true;
        }
    }

    if (opcode.up) {
        new_base = ir.add(address, IRConstant{bytes}, false);
    } else {
        opcode.pre = !opcode.pre;
        address = ir.sub(address, IRConstant{bytes}, false);
        new_base = ir.move(address, false);
    }

    emit_advance_pc();

    // TODO: make sure this is correct
    // stm armv4: store old base if rb is first in rlist, otherwise store new base
    // stm armv5: always store old base
    if (opcode.writeback && !opcode.load) {
        if ((jit.arch == Arch::ARMv4) && (first != opcode.rn)) {
            ir.store_gpr(opcode.rn, new_base);
        }
    }

    bool user_switch_mode = opcode.psr && (!opcode.load || !opcode.r15_in_rlist);
    if (user_switch_mode) {
        logger.todo("Translator: handle user switch mode");
    }

    for (int i = first; i < 16; i++) {
        if (!(opcode.rlist & (1 << i))) {
            continue;
        }

        if (opcode.pre) {
            address = ir.add(address, IRConstant{4}, false);
        }

        if (opcode.load) {
            auto data = ir.memory_read(address, AccessSize::Word, AccessType::Aligned);
            ir.store_gpr(static_cast<GPR>(i), data);
        } else {
            auto data = ir.load_gpr(static_cast<GPR>(i));
            ir.memory_write(address, data, AccessSize::Word, AccessType::Aligned);
        }

        if (!opcode.pre) {
            address = ir.add(address, IRConstant{4}, false);
        } 
    }

    if (opcode.writeback) {
        // TODO: make sure this is correct
        // ldm armv4: writeback if rn is not in rlist
        // ldm armv5: writeback if rn is only register or not the last register in rlist
        if (opcode.load) {
            if (jit.arch == Arch::ARMv5) {
                if ((opcode.rlist == (1 << opcode.rn)) || !((opcode.rlist >> opcode.rn) == 1)) {
                    ir.store_gpr(opcode.rn, new_base);
                }
            } else {
                if (!(opcode.rlist & (1 << opcode.rn))) {
                    ir.store_gpr(opcode.rn, new_base);
                }
            }
        } else {
            ir.store_gpr(opcode.rn, new_base);
        }
    } 

    if (user_switch_mode) {
        logger.todo("Translator: handle user switch mode");
    }

    if (opcode.load && opcode.r15_in_rlist) {
        logger.todo("Translator: handle ldm r15");
    }

    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_single_data_transfer() {
    auto opcode = ARMSingleDataTransfer::decode(instruction);
    IRValue op2;
    IRVariable address = ir.load_gpr(opcode.rn);
    bool do_writeback = !opcode.load || opcode.rd != opcode.rn;

    if (opcode.imm) {
        op2 = IRConstant{opcode.rhs.imm};
    } else {
        logger.todo("Translator: arm_single_data_transfer handle barrel shifter");
    }

    if (!opcode.up) {
        op2 = ir.multiply(op2, IRConstant{static_cast<u32>(-1)}, false);
    }

    if (opcode.pre) {
        address = ir.add(address, op2, false);
    }

    emit_advance_pc();

    if (opcode.load) {
        IRVariable dst;
        if (opcode.byte) {
            dst = ir.memory_read(address, AccessSize::Byte, AccessType::Aligned);
        } else {
            dst = ir.memory_read(address, AccessSize::Word, AccessType::Unaligned);
        }

        ir.store_gpr(opcode.rd, dst);
    } else {
        IRVariable src = ir.load_gpr(opcode.rd);
        if (opcode.byte) {
            ir.memory_write(address, src, AccessSize::Byte, AccessType::Aligned);
        } else {
            ir.memory_write(address, src, AccessSize::Word, AccessType::Aligned);
        }
    }

    if (do_writeback) {
        if (!opcode.pre) {
            auto base = ir.load_gpr(opcode.rn);
            auto new_base = ir.add(base, op2, false);
            ir.store_gpr(opcode.rn, new_base);
        } else if (opcode.writeback) {
            ir.store_gpr(opcode.rn, address);
        }
    }

    if (opcode.load && opcode.rd == 15) {
        logger.todo("Translator: handle arm_single_data_transfer pc write");
    }

    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_data_processing() {
    auto opcode = ARMDataProcessing::decode(instruction);

    // TODO: only some opcodes rely on the barrel shifter to update the carry flag,
    // so later only emit code to set/clear the carry flag if necessary.
    // right now the carry flag would just get overwritten by opcodes like sub, rsb, etc
    bool carry_done_in_opcode = opcode.opcode == ARMDataProcessing::Opcode::ADC ||
        opcode.opcode == ARMDataProcessing::Opcode::SBC ||
        opcode.opcode == ARMDataProcessing::Opcode::RSC;

    bool is_comparison = opcode.opcode == ARMDataProcessing::Opcode::TST ||
        opcode.opcode == ARMDataProcessing::Opcode::TEQ ||
        opcode.opcode == ARMDataProcessing::Opcode::CMP ||
        opcode.opcode == ARMDataProcessing::Opcode::CMN;

    bool set_carry = opcode.set_flags && !carry_done_in_opcode;
    bool update_flags = opcode.set_flags && (opcode.rd != GPR::PC || is_comparison);
    bool early_advance_pc = false;
    IRValue op2;

    if (opcode.imm) {
        op2 = IRConstant{opcode.rhs.imm.rotated};

        if (set_carry && opcode.rhs.imm.shift != 0) {
            ir.update_flag(Flags::C, common::get_bit<31>(opcode.rhs.imm.rotated));
            ir.store_flags(Flags::C);
        }
    } else {
        IRValue amount;
        op2 = ir.load_gpr(opcode.rhs.reg.rm);

        if (opcode.rhs.reg.imm) {
            amount = IRConstant{opcode.rhs.reg.amount.imm};
        } else {
            amount = ir.load_gpr(opcode.rhs.reg.amount.rs);
            emit_advance_pc();
            early_advance_pc = true;
        }

        op2 = emit_barrel_shifter(op2, opcode.rhs.reg.shift_type, amount, set_carry);
        if (set_carry) {
            ir.store_flags(Flags::C);
        }
    }

    switch (opcode.opcode) {
    case ARMDataProcessing::Opcode::AND: {
        IRValue op1 = ir.load_gpr(opcode.rn);
        auto dst = ir.and_(op1, op2, opcode.set_flags);
        ir.store_gpr(opcode.rd, dst);
        break;
    }
    case ARMDataProcessing::Opcode::EOR: {
        IRValue op1 = ir.load_gpr(opcode.rn);
        auto dst = ir.exclusive_or(op1, op2, opcode.set_flags);
        ir.store_gpr(opcode.rd, dst);
        break;
    }
    case ARMDataProcessing::Opcode::SUB: {
        IRValue op1 = ir.load_gpr(opcode.rn);
        auto dst = ir.sub(op1, op2, opcode.set_flags);
        ir.store_gpr(opcode.rd, dst);
        break;
    }
    case ARMDataProcessing::Opcode::RSB:
        logger.todo("Translator: handle rsb");
        break;
    case ARMDataProcessing::Opcode::ADD: {
        IRValue op1 = ir.load_gpr(opcode.rn);
        auto dst = ir.add(op1, op2, opcode.set_flags);
        ir.store_gpr(opcode.rd, dst);
        break;
    }
    case ARMDataProcessing::Opcode::ADC: {
        IRValue op1 = ir.load_gpr(opcode.rn);
        auto dst = ir.add_carry(op1, op2, opcode.set_flags);
        ir.store_gpr(opcode.rd, dst);
        break;
    }
    case ARMDataProcessing::Opcode::SBC:
        logger.todo("Translator: handle sbc");
        break;
    case ARMDataProcessing::Opcode::RSC:
        logger.todo("Translator: handle rsc");
        break;
    case ARMDataProcessing::Opcode::TST: {
        IRValue op1 = ir.load_gpr(opcode.rn);
        ir.test(op1, op2);
        break;
    }
    case ARMDataProcessing::Opcode::TEQ:
        logger.todo("Translator: handle teq");
        break;
    case ARMDataProcessing::Opcode::CMP: {
        IRValue op1 = ir.load_gpr(opcode.rn);
        ir.compare(op1, op2);
        break;
    }
    case ARMDataProcessing::Opcode::CMN: {
        IRValue op1 = ir.load_gpr(opcode.rn);
        ir.compare_negate(op1, op2);
        break;
    }
    case ARMDataProcessing::Opcode::ORR: {
        IRValue op1 = ir.load_gpr(opcode.rn);
        auto dst = ir.or_(op1, op2, opcode.set_flags);
        ir.store_gpr(opcode.rd, dst);
        break;
    }
    case ARMDataProcessing::Opcode::MOV: {
        auto dst = ir.move(op2, opcode.set_flags);
        ir.store_gpr(opcode.rd, dst);
        break;
    }
    case ARMDataProcessing::Opcode::BIC: {
        IRValue op1 = ir.load_gpr(opcode.rn);
        auto dst = ir.bic(op1, op2, opcode.set_flags);
        ir.store_gpr(opcode.rd, dst);
        break;
    }
    case ARMDataProcessing::Opcode::MVN: {
        auto dst = ir.move_negate(op2, opcode.set_flags);
        ir.store_gpr(opcode.rd, dst);
        break;
    }
    }

    if (update_flags) {
        std::array<Flags, 16> flags{
            Flags::NZ, Flags::NZ, Flags::NZCV, Flags::NZCV,
            Flags::NZCV, Flags::NZCV, Flags::NZCV, Flags::NZCV,
            Flags::NZ, Flags::NZ, Flags::NZCV, Flags::NZCV,
            Flags::NZ, Flags::NZ, Flags::NZ, Flags::NZ
        };

        ir.store_flags(flags[static_cast<int>(opcode.opcode)]);
    }

    if (opcode.rd == 15 && opcode.set_flags) {
        emit_copy_spsr_to_cpsr();
    }

    if (opcode.rd == 15 && !is_comparison) {
        if (opcode.set_flags) {
            logger.todo("Translator: handle pc write in data processing with set flags (t bit might change)");
        }

        return BlockStatus::Break;
    } else if (!early_advance_pc) {
        emit_advance_pc();
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