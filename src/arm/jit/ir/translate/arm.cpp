#include "common/logger.h"
#include "common/bits.h"
#include "arm/instructions.h"
#include "arm/jit/ir/translator.h"
#include "arm/jit/ir/value.h"
#include "arm/jit/jit.h"

namespace arm {

Translator::BlockStatus Translator::arm_branch_link_maybe_exchange() {
    if (static_cast<Condition>(common::get_field<28, 4>(instruction)) != Condition::NV) {
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
    if (jit.arch == Arch::ARMv4) {
        logger.todo("Translator: arm_branch_link_exchange_register executed by arm7");
    }

    auto opcode = ARMBranchExchange::decode(instruction);
    auto address = ir.load_gpr(opcode.rm);
    
    ir.link();
    ir.branch_exchange(address, ExchangeType::Bit0);
    return BlockStatus::Break;
}

Translator::BlockStatus Translator::arm_single_data_swap() {
    logger.todo("Translator: handle arm_single_data_swap");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_multiply() {
    auto opcode = ARMMultiply::decode(instruction);

    if (opcode.rd == 15) {
        logger.todo("handle rd == 15 in arm_multiply");
    }

    auto op1 = ir.load_gpr(opcode.rm);
    auto op2 = ir.load_gpr(opcode.rs);
    auto result = ir.multiply(op1, op2);
    
    if (opcode.accumulate) {
        auto op3 = ir.load_gpr(opcode.rn);
        result = ir.add(result, op3);
    }

    if (opcode.set_flags) {
        ir.store_nz(result);
    }

    ir.store_gpr(opcode.rd, result);
    ir.advance_pc();
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_saturating_add_subtract() {
    logger.todo("Translator: handle arm_saturating_add_subtract");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_multiply_long() {
    // auto opcode = ARMMultiplyLong::decode(instruction);
    // s64 result = 0;

    // if (opcode.sign) {
    //     result = common::sign_extend<s64, 32>(state.gpr[opcode.rm]) * common::sign_extend<s64, 32>(state.gpr[opcode.rs]);
    // } else {
    //     result = static_cast<s64>(static_cast<u64>(state.gpr[opcode.rm]) * static_cast<u64>(state.gpr[opcode.rs]));
    // }

    // if (opcode.accumulate) {
    //     result += static_cast<s64>((static_cast<u64>(state.gpr[opcode.rdhi]) << 32) | static_cast<u64>(state.gpr[opcode.rdlo]));
    // }

    // if (opcode.set_flags) {
    //     state.cpsr.n = result >> 63;
    //     state.cpsr.z = result == 0;
    // }

    // state.gpr[opcode.rdhi] = result >> 32;
    // state.gpr[opcode.rdlo] = result & 0xffffffff;
    // ir.advance_pc();
    logger.todo("arm_multiply_long");
    
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_halfword_data_transfer() {
    auto opcode = ARMHalfwordDataTransfer::decode(instruction);

    if (opcode.rd == 15) {
        logger.todo("Translator: handle rd == 15 in arm_halfword_data_transfer");
    }

    IRValue op2;
    auto address = ir.load_gpr(opcode.rn);
    bool do_writeback = !opcode.load || opcode.rd != opcode.rn;

    if (opcode.imm) {
        op2 = ir.constant(opcode.rhs.imm);
    } else {
        op2 = ir.load_gpr(opcode.rhs.rm);
    }

    if (!opcode.up) {
        op2 = ir.multiply(op2, ir.constant(static_cast<u32>(-1)));
    }

    if (opcode.pre) {
        address = ir.add(address, op2);
    }

    ir.advance_pc();

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
            auto src = ir.load_gpr(opcode.rd);
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
            auto new_base = ir.add(base, op2);
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
    auto value_masked = ir.bitwise_and(value, ir.constant(opcode.mask));
    IRVariable psr;

    if (opcode.spsr) {
        psr = ir.load_spsr();
    } else {
        psr = ir.load_cpsr();
    }

    auto psr_masked = ir.bitwise_and(psr, ir.constant(~opcode.mask));
    auto psr_new = ir.bitwise_or(psr_masked, value_masked);

    ir.advance_pc();

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
    auto opcode = ARMStatusStore::decode(instruction);
    auto value = ir.constant(opcode.rhs.rotated);;
    auto value_masked = ir.bitwise_and(value, ir.constant(opcode.mask));
    IRVariable psr;

    if (opcode.spsr) {
        psr = ir.load_spsr();
    } else {
        psr = ir.load_cpsr();
    }

    auto psr_masked = ir.bitwise_and(psr, ir.constant(~opcode.mask));
    auto psr_new = ir.bitwise_or(psr_masked, value_masked);

    ir.advance_pc();

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
        new_base = ir.add(address, ir.constant(bytes));
    } else {
        opcode.pre = !opcode.pre;
        address = ir.subtract(address, ir.constant(bytes));
        new_base = ir.copy(address);
    }

    ir.advance_pc();

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
            address = ir.add(address, ir.constant(4));
        }

        if (opcode.load) {
            auto data = ir.memory_read(address, AccessSize::Word, AccessType::Aligned);
            ir.store_gpr(static_cast<GPR>(i), data);
        } else {
            auto data = ir.load_gpr(static_cast<GPR>(i));
            ir.memory_write(address, data, AccessSize::Word, AccessType::Aligned);
        }

        if (!opcode.pre) {
            address = ir.add(address, ir.constant(4));
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
    auto address = ir.load_gpr(opcode.rn);
    bool do_writeback = !opcode.load || opcode.rd != opcode.rn;

    if (opcode.imm) {
        op2 = ir.constant(opcode.rhs.imm);
    } else {
        auto src = ir.load_gpr(opcode.rhs.reg.rm);
        auto pair = ir.barrel_shifter(src, opcode.rhs.reg.shift_type, ir.constant(opcode.rhs.reg.amount));
        op2 = pair.first;
    }

    if (!opcode.up) {
        op2 = ir.multiply(op2, ir.constant(static_cast<u32>(-1)));
    }

    if (opcode.pre) {
        address = ir.add(address, op2);
    }

    ir.advance_pc();

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
            auto new_base = ir.add(base, op2);
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
        IRValue amount;
        op2 = ir.load_gpr(opcode.rhs.reg.rm);

        if (opcode.rhs.reg.imm) {
            amount = ir.constant(opcode.rhs.reg.amount.imm);
        } else {
            amount = ir.load_gpr(opcode.rhs.reg.amount.rs);
            ir.advance_pc();
            early_advance_pc = true;
        }

        auto pair = ir.barrel_shifter(op2, opcode.rhs.reg.shift_type, amount);
        op2 = pair.first;
        if (set_carry) {
            ir.store_flag(Flag::C, pair.second);
        }
    }

    IRValue op1;
    IRVariable result;

    if (uses_op1) {
        op1 = ir.load_gpr(opcode.rn);
    }

    switch (opcode.opcode) {
    case ARMDataProcessing::Opcode::AND:
        result = ir.bitwise_and(op1, op2);
        if (update_flags) {
            ir.store_nz(result);
        }
        
        break;
    case ARMDataProcessing::Opcode::EOR:
        result = ir.bitwise_exclusive_or(op1, op2);
        if (update_flags) {
            ir.store_nz(result);
        }
        
        break;
    case ARMDataProcessing::Opcode::SUB:
        result = ir.subtract(op1, op2);
        if (update_flags) {
            ir.store_nz(result);
            ir.store_sub_cv(op1, op2, result);
        }
        
        break;
    case ARMDataProcessing::Opcode::RSB:
        result = ir.subtract(op2, op1);
        if (update_flags) {
            ir.store_nz(result);
            ir.store_sub_cv(op2, op1, result);
        }
        
        break;
    case ARMDataProcessing::Opcode::ADD:
        result = ir.add(op1, op2);
        if (update_flags) {
            ir.store_nz(result);
            ir.store_add_cv(op1, op2, result);
        }
        
        break;
    case ARMDataProcessing::Opcode::ADC:
        result = ir.add(ir.add(op1, op2), ir.load_flag(Flag::C));
        if (update_flags) {
            ir.store_nz(result);
            ir.store_adc_cv(op1, op2, result);
        }

        break;
    case ARMDataProcessing::Opcode::SBC:
        result = ir.subtract(ir.subtract(op1, op2), ir.bitwise_exclusive_or(ir.load_flag(Flag::C), ir.constant(1)));
        if (update_flags) {
            ir.store_nz(result);
            ir.store_sbc_cv(op1, op2, result);
        }

        break;
    case ARMDataProcessing::Opcode::RSC:
        result = ir.subtract(ir.subtract(op2, op1), ir.bitwise_exclusive_or(ir.load_flag(Flag::C), ir.constant(1)));
        if (update_flags) {
            ir.store_nz(result);
            ir.store_sbc_cv(op2, op1, result);
        }

        break;
    case ARMDataProcessing::Opcode::TST: {
        auto result = ir.bitwise_and(op1, op2);
        ir.store_nz(result);
        break;
    }
    case ARMDataProcessing::Opcode::TEQ: {
        auto result = ir.bitwise_exclusive_or(op1, op2);
        ir.store_nz(result);
        break;
    }
    case ARMDataProcessing::Opcode::CMP: {
        auto result = ir.subtract(op1, op2);
        ir.store_nz(result);
        ir.store_sub_cv(op1, op2, result);
        break;
    }
    case ARMDataProcessing::Opcode::CMN: {
        auto result = ir.add(op1, op2);
        ir.store_nz(result);
        ir.store_add_cv(op1, op2, result);
        break;
    }
    case ARMDataProcessing::Opcode::ORR:
        result = ir.bitwise_or(op1, op2);
        if (update_flags) {
            ir.store_nz(result);
        }
        
        break;
    case ARMDataProcessing::Opcode::MOV:
        result = ir.copy(op2);
        if (update_flags) {
            ir.store_nz(result);
        }

        break;
    case ARMDataProcessing::Opcode::BIC:
        result = ir.bitwise_and(op1, ir.bitwise_not(op2));
        if (update_flags) {
            ir.store_nz(result);
        }
        
        break;
    case ARMDataProcessing::Opcode::MVN:
        result = ir.bitwise_not(op2);
        if (update_flags) {
            ir.store_nz(result);
        }

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