#include "common/logger.h"
#include "common/bits.h"
#include "arm/instructions.h"
#include "arm/jit/ir/translator.h"
#include "arm/jit/ir/value.h"
#include "arm/jit/jit.h"
#include "arm/disassembler/disassembler.h"

namespace arm {

static Disassembler disassembler;

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
    if (jit.arch == Arch::ARMv4) {
        LOG_TODO("Translator: handle arm_count_leading_zeroes on armv4");
    }

    auto opcode = ARMCountLeadingZeroes::decode(instruction);

    if (opcode.rd == 15) {
        LOG_TODO("handle pc in arm_count_leading_zeroes");
    }

    auto src = ir.load_gpr(opcode.rm);
    auto result = ir.count_leading_zeroes(src);

    ir.store_gpr(opcode.rd, result);
    ir.advance_pc();
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_branch_link() {
    auto opcode = ARMBranchLink::decode(instruction);
    auto instruction_size = ir.basic_block.location.get_instruction_size();
    if (opcode.link) {
        ir.link();
    }

    ir.branch(ir.imm32(ir.basic_block.current_address + (4 * instruction_size) + opcode.offset));
    return BlockStatus::Break;
}

Translator::BlockStatus Translator::arm_branch_link_exchange() {
    if (jit.arch == Arch::ARMv4) {
        LOG_TODO("Translator: arm_branch_link_exchange executed by arm7");
    }

    auto opcode = ARMBranchLinkExchange::decode(instruction);
    auto instruction_size = ir.basic_block.location.get_instruction_size();
    ir.link();
    ir.store_flag(Flag::T, ir.imm1(true));
    ir.branch(ir.imm32(ir.basic_block.current_address + (2 * instruction_size) + opcode.offset + 4));
    return BlockStatus::Break;
}

Translator::BlockStatus Translator::arm_branch_link_exchange_register() {
    if (jit.arch == Arch::ARMv4) {
        LOG_TODO("Translator: arm_branch_link_exchange_register executed by arm7");
    }

    auto opcode = ARMBranchExchange::decode(instruction);
    auto address = ir.load_gpr(opcode.rm);
    
    ir.link();
    ir.branch_exchange(address, ExchangeType::Bit0);
    return BlockStatus::Break;
}

Translator::BlockStatus Translator::arm_single_data_swap() {
    auto opcode = ARMSingleDataSwap::decode(instruction);

    if (opcode.rd == 15) {
        LOG_TODO("pc write in arm_single_data_swap");
    }

    auto address = ir.load_gpr(opcode.rn);
    auto data = ir.load_gpr(opcode.rm);
    TypedValue<Type::U32> result;

    if (opcode.byte) {
        result = ir.memory_read(address, AccessSize::Byte, AccessType::Aligned);
        ir.memory_write_byte(address, ir.truncate_byte(data));
    } else {
        result = ir.memory_read(address, AccessSize::Word, AccessType::Unaligned);
        ir.memory_write_word(address, data);
    }

    ir.store_gpr(opcode.rd, result);
    ir.advance_pc();
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_multiply() {
    auto opcode = ARMMultiply::decode(instruction);

    if (opcode.rd == 15) {
        LOG_TODO("handle rd == 15 in arm_multiply");
    }

    auto op1 = ir.load_gpr(opcode.rm);
    auto op2 = ir.load_gpr(opcode.rs);
    auto result = ir.multiply(op1, op2);
    
    if (opcode.accumulate) {
        auto op3 = ir.load_gpr(opcode.rn);
        result = ir.add(result, op3);
    }

    ir.store_gpr(opcode.rd, result);
    ir.advance_pc();

    if (opcode.set_flags) {
        ir.store_nz(result);
        return BlockStatus::FlagsChanged;
    }

    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_saturating_add_subtract() {
    LOG_TODO("Translator: handle arm_saturating_add_subtract");
    return BlockStatus::Break;
}

Translator::BlockStatus Translator::arm_multiply_long() {
    auto opcode = ARMMultiplyLong::decode(instruction);
    auto op1 = ir.load_gpr(opcode.rm);
    auto op2 = ir.load_gpr(opcode.rs);
    auto result = ir.multiply_long(op1, op2, opcode.sign);

    if (opcode.accumulate) {
        auto op3 = ir.load_gpr(opcode.rdhi);
        auto op4 = ir.load_gpr(opcode.rdlo);
        result = ir.add_long(ir.pair(TypedValue<Type::U32>{result.first}, TypedValue<Type::U32>{result.second}), ir.pair(op3, op4));
    }

    ir.store_gpr(opcode.rdhi, TypedValue<Type::U32>{result.first});
    ir.store_gpr(opcode.rdlo, TypedValue<Type::U32>{result.second});
    ir.advance_pc();

    if (opcode.set_flags) {
        ir.store_nz_long(result);
        return BlockStatus::FlagsChanged;
    }

    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_halfword_data_transfer() {
    auto opcode = ARMHalfwordDataTransfer::decode(instruction);

    if (opcode.rd == 15) {
        LOG_TODO("Translator: handle rd == 15 in arm_halfword_data_transfer");
    }

    TypedValue<Type::U32> op2;
    auto address = ir.load_gpr(opcode.rn);
    bool do_writeback = !opcode.load || opcode.rd != opcode.rn;

    if (opcode.imm) {
        op2 = ir.imm32(opcode.rhs.imm);
    } else {
        op2 = ir.load_gpr(opcode.rhs.rm);
    }

    if (!opcode.up) {
        op2 = ir.multiply(op2, ir.imm32(static_cast<u32>(-1)));
    }

    if (opcode.pre) {
        address = ir.add(address, op2);
    }

    ir.advance_pc();

    if (opcode.half && opcode.sign) {
        if (opcode.load) {
            if (jit.arch == Arch::ARMv4) {
                auto unaligned = ir.bitwise_and(address, ir.imm32(0x1));
                auto shift = ir.multiply(unaligned, ir.imm32(0x8));
                auto data = ir.memory_read(address, AccessSize::Half, AccessType::Aligned);
                auto sign_extended = ir.sign_extend_half(data);
                auto dst = ir.arithmetic_shift_right(sign_extended, ir.truncate_byte(shift));
                ir.store_gpr(opcode.rd, dst);
            } else {
                auto data = ir.memory_read(address, AccessSize::Half, AccessType::Aligned);
                auto dst = ir.sign_extend_half(data);
                ir.store_gpr(opcode.rd, dst);
            }
        } else if (jit.arch == Arch::ARMv5) {
            LOG_TODO("Translator: handle strd");
        }
    } else if (opcode.half) {
        if (opcode.load) {
            auto data = ir.memory_read(address, AccessSize::Half, AccessType::Aligned);

            if (jit.arch == Arch::ARMv4) {
                auto unaligned = ir.bitwise_and(address, ir.imm32(0x1));
                auto shift = ir.multiply(unaligned, ir.imm32(0x8));
                auto dst = ir.rotate_right(data, ir.truncate_byte(shift));
                ir.store_gpr(opcode.rd, dst);
            } else {
                ir.store_gpr(opcode.rd, data);
            }
        } else {
            auto src = ir.load_gpr(opcode.rd);
            ir.memory_write_half(address, ir.truncate_half(src));
        }
    } else if (opcode.sign) {
        if (opcode.load) {
            auto dst = ir.sign_extend_byte(ir.memory_read(address, AccessSize::Byte, AccessType::Aligned));
            ir.store_gpr(opcode.rd, dst);
        } else if (jit.arch == Arch::ARMv5) {
            LOG_TODO("Translator: handle ldrd");
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
    auto opcode = ARMStatusLoad::decode(instruction);

    if (opcode.rd == 15) {
        LOG_TODO("pc write in arm_status_load");
    }

    if (opcode.spsr) {
        ir.store_gpr(opcode.rd, ir.load_spsr());
    } else {
        ir.store_gpr(opcode.rd, ir.load_cpsr());
    }

    ir.advance_pc();
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_status_store_register() {
    auto opcode = ARMStatusStore::decode(instruction);
    auto value = ir.load_gpr(opcode.rhs.rm);
    auto value_masked = ir.bitwise_and(value, ir.imm32(opcode.mask));
    TypedValue<Type::U32> psr;

    if (opcode.spsr) {
        psr = ir.load_spsr();
    } else {
        psr = ir.load_cpsr();
    }

    auto psr_masked = ir.bitwise_and(psr, ir.imm32(~opcode.mask));
    auto psr_new = ir.bitwise_or(psr_masked, value_masked);

    ir.advance_pc();

    if (opcode.spsr) {
        ir.store_spsr(psr_new);
    } else {
        ir.store_cpsr(psr_new);

        if (opcode.mask & 0xff) {
            return BlockStatus::Break;
        }

        if (opcode.mask & 0xff000000) {
            return BlockStatus::FlagsChanged;
        }
    }

    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_status_store_immediate() {
    auto opcode = ARMStatusStore::decode(instruction);
    auto value = ir.imm32(opcode.rhs.rotated);;
    auto value_masked = ir.bitwise_and(value, ir.imm32(opcode.mask));
    TypedValue<Type::U32> psr;

    if (opcode.spsr) {
        psr = ir.load_spsr();
    } else {
        psr = ir.load_cpsr();
    }

    auto psr_masked = ir.bitwise_and(psr, ir.imm32(~opcode.mask));
    auto psr_new = ir.bitwise_or(psr_masked, value_masked);

    ir.advance_pc();

    if (opcode.spsr) {
        ir.store_spsr(psr_new);
    } else {
        ir.store_cpsr(psr_new);

        if (opcode.mask & 0xff) {
            return BlockStatus::Break;
        }

        if (opcode.mask & 0xff000000) {
            return BlockStatus::FlagsChanged;
        }
    }

    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_block_data_transfer() {
    auto opcode = ARMBlockDataTransfer::decode(instruction);
    auto address = ir.load_gpr(opcode.rn);
    auto first = 0;
    u32 bytes = 0;
    TypedValue<Type::U32> new_base;

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
        new_base = ir.add(address, ir.imm32(bytes));
    } else {
        opcode.pre = !opcode.pre;
        address = ir.subtract(address, ir.imm32(bytes));
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
    auto mode = user_switch_mode ? Mode::USR : ir.basic_block.location.get_mode();

    for (int i = first; i < 16; i++) {
        if (!(opcode.rlist & (1 << i))) {
            continue;
        }

        if (opcode.pre) {
            address = ir.add(address, ir.imm32(4));
        }

        if (opcode.load) {
            auto data = ir.memory_read(address, AccessSize::Word, AccessType::Aligned);
            ir.store_gpr(static_cast<GPR>(i), mode, data);
        } else {
            auto data = ir.load_gpr(static_cast<GPR>(i), mode);
            ir.memory_write_word(address, data);
        }

        if (!opcode.pre) {
            address = ir.add(address, ir.imm32(4));
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

    if (user_switch_mode && opcode.load && opcode.r15_in_rlist) {
        LOG_TODO("Translator: handle user switch mode r15");
    }

    if (opcode.load && opcode.r15_in_rlist) {
        if (jit.arch == Arch::ARMv5) {
            auto address = ir.load_gpr(GPR::PC);
            ir.branch_exchange(address, ExchangeType::Bit0);
        } else {
            ir.flush_pipeline();
        }
        
        return BlockStatus::Break;
    }

    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_single_data_transfer() {
    auto opcode = ARMSingleDataTransfer::decode(instruction);
    TypedValue<Type::U32> op2;
    auto address = ir.load_gpr(opcode.rn);
    bool do_writeback = !opcode.load || opcode.rd != opcode.rn;

    if (opcode.imm) {
        op2 = ir.imm32(opcode.rhs.imm);
    } else {
        auto src = ir.load_gpr(opcode.rhs.reg.rm);
        auto pair = ir.barrel_shifter(src, opcode.rhs.reg.shift_type, ir.imm8(opcode.rhs.reg.amount));
        op2 = TypedValue<Type::U32>{pair.first};
    }

    if (!opcode.up) {
        op2 = ir.multiply(op2, ir.imm32(static_cast<u32>(-1)));
    }

    if (opcode.pre) {
        address = ir.add(address, op2);
    }

    ir.advance_pc();

    if (opcode.load) {
        TypedValue<Type::U32> dst;
        if (opcode.byte) {
            dst = ir.memory_read(address, AccessSize::Byte, AccessType::Aligned);
        } else {
            dst = ir.memory_read(address, AccessSize::Word, AccessType::Unaligned);
        }

        ir.store_gpr(opcode.rd, dst);
    } else {
        TypedValue<Type::U32> src = ir.load_gpr(opcode.rd);
        if (opcode.byte) {
            ir.memory_write_byte(address, ir.truncate_byte(src));
        } else {
            ir.memory_write_word(address, src);
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
        if (jit.arch == Arch::ARMv5) {
            auto address = ir.load_gpr(opcode.rd);
            ir.branch_exchange(address, ExchangeType::Bit0);
        } else {
            ir.flush_pipeline();
        }

        return BlockStatus::Break;
    }

    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_data_processing() {
    auto opcode = ARMDataProcessing::decode(instruction);
    TypedValue<Type::U32> op2;
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
        op2 = ir.imm32(opcode.rhs.imm.rotated);

        if (set_carry && opcode.rhs.imm.shift != 0) {
            ir.store_flag(Flag::C, ir.imm1(opcode.rhs.imm.rotated >> 31));
        }
    } else {
        TypedValue<Type::U8> amount;
        if (opcode.rhs.reg.imm) {
            amount = ir.imm8(opcode.rhs.reg.amount.imm);
        } else {
            amount = ir.truncate_byte(ir.load_gpr(opcode.rhs.reg.amount.rs));
            ir.advance_pc();
            early_advance_pc = true;
        }

        auto src = ir.load_gpr(opcode.rhs.reg.rm);
        auto pair = ir.barrel_shifter(src, opcode.rhs.reg.shift_type, amount);
        op2 = TypedValue<Type::U32>{pair.first};
        if (set_carry) {
            ir.store_flag(Flag::C, TypedValue<Type::U1>{pair.second});
        }
    }

    TypedValue<Type::U32> op1;
    TypedValue<Type::U32> result;

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
        result = ir.add_with_carry(op1, op2, ir.load_flag(Flag::C));
        if (update_flags) {
            ir.store_nz(result);
            ir.store_adc_cv(op1, op2, result);
        }

        break;
    case ARMDataProcessing::Opcode::SBC:
        result = ir.subtract_with_carry(op1, op2, ir.load_flag(Flag::C));
        if (update_flags) {
            ir.store_nz(result);
            ir.store_sbc_cv(op1, op2, result);
        }

        break;
    case ARMDataProcessing::Opcode::RSC:
        result = ir.subtract_with_carry(op2, op1, ir.load_flag(Flag::C));
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
            ir.branch_exchange(result, ExchangeType::ThumbBit);
        } else {
            ir.flush_pipeline();
        }

        return BlockStatus::Break;
    } else if (!early_advance_pc) {
        ir.advance_pc();
    }

    if (opcode.set_flags) {
        return BlockStatus::FlagsChanged;
    }

    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_coprocessor_register_transfer() {
    auto opcode = ARMCoprocessorRegisterTransfer::decode(instruction);

    // TODO: handle this in a nicer way
    if (jit.arch == Arch::ARMv4 && opcode.cp == 14) {
        LOG_TODO("Translator: mrc cp14 on arm7");
    } else if ((jit.arch == Arch::ARMv4 && opcode.cp == 15) || (jit.arch == Arch::ARMv5 && opcode.cp == 14)) {
        LOG_TODO("Translator: undefined exception arm_coprocessor_register_transfer");
    }

    if (opcode.rd == 15) {
        LOG_ERROR("Interpreter: handle rd == 15 in arm_coprocessor_register_transfer");
    }

    if (opcode.load) {
        ir.store_gpr(opcode.rd, ir.load_coprocessor(opcode.crn, opcode.crm, opcode.cp));
    } else {
        ir.store_coprocessor(opcode.crn, opcode.crm, opcode.cp, ir.load_gpr(opcode.rd));
    }

    ir.advance_pc();

    if (!opcode.load && jit.coprocessor.has_side_effects(opcode.crn, opcode.crm, opcode.cp)) {
        return BlockStatus::Break;
    }

    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_software_interrupt() {
    // TODO: make sure the exception base when compiling doesn't change later
    auto exception_address = jit.coprocessor.get_exception_base() + 0x08;
    auto instruction_size = ir.basic_block.location.get_instruction_size();
    auto cpsr = ir.load_cpsr();
    ir.store_spsr(cpsr, Mode::SVC);

    auto cpsr_cleared = ir.bitwise_and(cpsr, ir.imm32(~0x1f));
    auto cpsr_new = ir.bitwise_or(cpsr_cleared, ir.imm32(static_cast<u32>(Mode::SVC)));
    ir.store_cpsr(cpsr_new);
    ir.store_flag(Flag::I, ir.imm1(true));

    // we can't use branch or link helpers here since we need to specifically write to svc registers
    ir.store_gpr(GPR::LR, Mode::SVC, ir.imm32(ir.basic_block.current_address + instruction_size));
    ir.store_gpr(GPR::PC, Mode::SVC, ir.imm32(exception_address + (2 * instruction_size)));
    return BlockStatus::Break;
}

Translator::BlockStatus Translator::arm_signed_multiply_accumulate_long() {
    LOG_TODO("Translator: handle arm_signed_multiply_accumulate_long");
    return BlockStatus::Break;
}

Translator::BlockStatus Translator::arm_signed_multiply_word() {
    LOG_TODO("Translator: handle arm_signed_multiply_word");
    return BlockStatus::Break;
}

Translator::BlockStatus Translator::arm_signed_multiply() {
    if (jit.arch == Arch::ARMv4) {
        LOG_TODO("Translator: handle armv4 arm_signed_multiply");
    }

    auto opcode = ARMSignedMultiply::decode(instruction);
    TypedValue<Type::U32> lhs;
    TypedValue<Type::U32> rhs;

    if (opcode.x) {
        lhs = ir.sign_extend_half(ir.logical_shift_right(ir.load_gpr(opcode.rm), ir.imm8(16)));
    } else {
        lhs = ir.sign_extend_half(ir.load_gpr(opcode.rm));
    }

    if (opcode.y) {
        rhs = ir.sign_extend_half(ir.logical_shift_right(ir.load_gpr(opcode.rs), ir.imm8(16)));
    } else {
        rhs = ir.sign_extend_half(ir.load_gpr(opcode.rs));
    }

    auto result = ir.multiply(lhs, rhs);

    if (opcode.accumulate) {
        auto operand = ir.load_gpr(opcode.rn);
        auto added = ir.add(result, operand);
        auto add_overflow = ir.add_overflow(result, operand, added);
        auto cpsr = ir.load_cpsr();
        auto cpsr_new = ir.set_bit(cpsr, add_overflow, ir.imm8(Flag::Q));
        
        ir.store_cpsr(cpsr_new);
        ir.store_gpr(opcode.rd, added);
    } else {
        ir.store_gpr(opcode.rd, result);
    }

    ir.advance_pc();
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::arm_breakpoint() {
    LOG_TODO("Translator: handle arm_breakpoint");
    return BlockStatus::Break;
}

} // namespace arm