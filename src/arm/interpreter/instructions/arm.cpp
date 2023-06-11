#include "common/bits.h"
#include "common/logger.h"
#include "arm/interpreter/interpreter.h"

namespace arm {

void Interpreter::arm_data_processing() {
    auto opcode = ARMDataProcessing::decode(instruction);
    u32 op1 = state.gpr[opcode.rn];
    u32 op2 = 0;
    bool carry = state.cpsr.c;

    if (opcode.imm) {
        op2 = opcode.rhs.imm.rotated;
        if (opcode.rhs.imm.shift != 0) {
            carry = op2 >> 31;
        }
    } else {
        int amount = 0;
        op2 = state.gpr[opcode.rhs.reg.rm];

        if (opcode.rhs.reg.imm) {
            amount = opcode.rhs.reg.amount.imm;
        } else {
            amount = state.gpr[opcode.rhs.reg.amount.rs] & 0xff;

            if (opcode.rn == 15) {
                op1 += 4;
            }

            if (opcode.rhs.reg.rm == 15) {
                op2 += 4;
            }
        }

        op2 = barrel_shifter(op2, opcode.rhs.reg.shift_type, amount, carry, opcode.rhs.reg.imm);
    }

    switch (opcode.opcode) {
    case ARMDataProcessing::Opcode::AND:
        state.gpr[opcode.rd] = alu_and(op1, op2, opcode.set_flags);
        if (opcode.set_flags) {
            state.cpsr.c = carry;
        }

        break;
    case ARMDataProcessing::Opcode::EOR:
        state.gpr[opcode.rd] = alu_eor(op1, op2, opcode.set_flags);
        if (opcode.set_flags) {
            state.cpsr.c = carry;
        }

        break;
    case ARMDataProcessing::Opcode::SUB:
        state.gpr[opcode.rd] = alu_sub(op1, op2, opcode.set_flags);
        break;
    case ARMDataProcessing::Opcode::RSB:
        state.gpr[opcode.rd] = alu_sub(op2, op1, opcode.set_flags);
        break;
    case ARMDataProcessing::Opcode::ADD:
        state.gpr[opcode.rd] = alu_add(op1, op2, opcode.set_flags);
        break;
    case ARMDataProcessing::Opcode::ADC:
        state.gpr[opcode.rd] = alu_adc(op1, op2, opcode.set_flags);
        break;
    case ARMDataProcessing::Opcode::SBC:
        state.gpr[opcode.rd] = alu_sbc(op1, op2, opcode.set_flags);
        break;
    case ARMDataProcessing::Opcode::RSC:
        state.gpr[opcode.rd] = alu_sbc(op2, op1, opcode.set_flags);
        break;
    case ARMDataProcessing::Opcode::TST:
        alu_tst(op1, op2);
        state.cpsr.c = carry;
        break;
    case ARMDataProcessing::Opcode::TEQ:
        alu_teq(op1, op2);
        state.cpsr.c = carry;
        break;
    case ARMDataProcessing::Opcode::CMP:
        alu_cmp(op1, op2);
        break;
    case ARMDataProcessing::Opcode::CMN:
        alu_cmn(op1, op2);
        break;
    case ARMDataProcessing::Opcode::ORR:
        state.gpr[opcode.rd] = alu_orr(op1, op2, opcode.set_flags);
        if (opcode.set_flags) {
            state.cpsr.c = carry;
        }

        break;
    case ARMDataProcessing::Opcode::MOV:
        state.gpr[opcode.rd] = alu_mov(op2, opcode.set_flags);
        if (opcode.set_flags) {
            state.cpsr.c = carry;
        }
        
        break;
    case ARMDataProcessing::Opcode::BIC:
        state.gpr[opcode.rd] = alu_bic(op1, op2, opcode.set_flags);
        if (opcode.set_flags) {
            state.cpsr.c = carry;
        }

        break;
    case ARMDataProcessing::Opcode::MVN:
        state.gpr[opcode.rd] = alu_mvn(op2, opcode.set_flags);
        if (opcode.set_flags) {
            state.cpsr.c = carry;
        }

        break;
    }

    if (opcode.rd == 15) {
        if (opcode.set_flags) {
            auto spsr = *state.spsr;
            set_mode(spsr.mode);
            state.cpsr = spsr;
        }

        if (opcode.opcode != ARMDataProcessing::Opcode::TST &&
            opcode.opcode != ARMDataProcessing::Opcode::TEQ &&
            opcode.opcode != ARMDataProcessing::Opcode::CMP &&
            opcode.opcode != ARMDataProcessing::Opcode::CMN
        ) {
            if (state.cpsr.t) {
                thumb_flush_pipeline();
            } else {
                arm_flush_pipeline();
            }
        }
    } else {
        state.gpr[15] += 4;
    }
}

void Interpreter::arm_multiply() {
    auto opcode = ARMMultiply::decode(instruction);
    u32 result = state.gpr[opcode.rm] * state.gpr[opcode.rs];

    if (opcode.accumulate) {
        result += state.gpr[opcode.rn];
    }

    if (opcode.set_flags) {
        set_nz(result);
    }

    state.gpr[opcode.rd] = result;
    state.gpr[15] += 4;
}

void Interpreter::arm_multiply_long() {
    auto opcode = ARMMultiplyLong::decode(instruction);
    s64 result = 0;

    if (opcode.sign) {
        result = common::sign_extend<s64, 32>(state.gpr[opcode.rm]) * common::sign_extend<s64, 32>(state.gpr[opcode.rs]);
    } else {
        result = static_cast<s64>(static_cast<u64>(state.gpr[opcode.rm]) * static_cast<u64>(state.gpr[opcode.rs]));
    }

    if (opcode.accumulate) {
        result += static_cast<s64>((static_cast<u64>(state.gpr[opcode.rdhi]) << 32) | static_cast<u64>(state.gpr[opcode.rdlo]));
    }

    if (opcode.set_flags) {
        state.cpsr.n = result >> 63;
        state.cpsr.z = result == 0;
    }

    state.gpr[opcode.rdhi] = result >> 32;
    state.gpr[opcode.rdlo] = result & 0xffffffff;
    state.gpr[15] += 4;
}

void Interpreter::arm_single_data_swap() {
    auto opcode = ARMSingleDataSwap::decode(instruction);
    u32 addr = state.gpr[opcode.rn];
    u32 data = 0;

    if (opcode.byte) {
        data = read_byte(addr);
        write_byte(addr, state.gpr[opcode.rm]);
    } else {
        data = read_word_rotate(addr);
        write_word(addr, state.gpr[opcode.rm]);
    }

    state.gpr[opcode.rd] = data;
    state.gpr[15] += 4;
}

void Interpreter::arm_count_leading_zeroes() {
    if (arch == Arch::ARMv4) {
        undefined_exception();
        return;
    }

    auto opcode = ARMCountLeadingZeroes::decode(instruction);
    state.gpr[opcode.rd] = common::countl_zeroes(state.gpr[opcode.rm]);
    state.gpr[15] += 4;
}

void Interpreter::arm_saturating_add_subtract() {
    if (arch == Arch::ARMv4) {
        undefined_exception();
        return;
    }

    auto opcode = ARMSaturatingAddSubtract::decode(instruction);
    u32 lhs = state.gpr[opcode.rm];
    u32 rhs = state.gpr[opcode.rn];

    if (opcode.rd == 15) {
        logger.todo("Interpreter: handle rd == 15 in arm_saturating_add_subtract");
    }

    if (opcode.double_rhs) {
        u32 result = rhs + rhs;
        if ((rhs ^ result) >> 31) {
            state.cpsr.q = true;
            result = 0x80000000 - (result >> 31);
        }

        rhs = result;
    }

    if (opcode.sub) {
        u32 result = lhs - rhs;
        if (calculate_sub_overflow(lhs, rhs, result)) {
            state.cpsr.q = true;
            result = 0x80000000 - (result >> 31);
        }

        state.gpr[opcode.rd] = result;
    } else {
        u32 result = lhs + rhs;
        if (calculate_add_overflow(lhs, rhs, result)) {
            state.cpsr.q = true;
            result = 0x80000000 - (result >> 31);
        }

        state.gpr[opcode.rd] = result;
    }

    state.gpr[15] += 4;
}

void Interpreter::arm_signed_multiply() {
    if (arch == Arch::ARMv4) {
        return;
    }

    auto opcode = ARMSignedMultiply::decode(instruction);
    s16 lhs;
    s16 rhs;

    if (opcode.x) {
        lhs = static_cast<s16>(state.gpr[opcode.rm] >> 16);
    } else {
        lhs = static_cast<s16>(state.gpr[opcode.rm]);
    }

    if (opcode.y) {
        rhs = static_cast<s16>(state.gpr[opcode.rs] >> 16);
    } else {
        rhs = static_cast<s16>(state.gpr[opcode.rs]);
    }

    u32 result = lhs * rhs;

    if (opcode.accumulate) {
        u32 operand = state.gpr[opcode.rn];
        state.gpr[opcode.rd] = result + operand;

        if (calculate_add_overflow(result, operand, state.gpr[opcode.rd])) {
            state.cpsr.q = true;
        }
    } else {
        state.gpr[opcode.rd] = result;
    }

    state.gpr[15] += 4;
}

void Interpreter::arm_signed_multiply_word() {
    if (arch == Arch::ARMv4) {
        return;
    }

    auto opcode = ARMSignedMultiplyWord::decode(instruction);
    u32 result;
    if (opcode.y) {
        result = (static_cast<s32>(state.gpr[opcode.rm]) * static_cast<s16>(state.gpr[opcode.rs] >> 16)) >> 16;
    } else {
        result = (static_cast<s32>(state.gpr[opcode.rm]) * static_cast<s16>(state.gpr[opcode.rs])) >> 16;
    }

    if (opcode.accumulate) {
        u32 operand = state.gpr[opcode.rn];
        state.gpr[opcode.rd] = result + operand;

        if (calculate_add_overflow(result, operand, state.gpr[opcode.rd])) {
            state.cpsr.q = true;
        }
    } else {
        state.gpr[opcode.rd] = result;
    }

    state.gpr[15] += 4;
}

void Interpreter::arm_signed_multiply_accumulate_long() {
    if (arch == Arch::ARMv4) {
        return;
    }

    auto opcode = ARMSignedMultiplyAccumulateLong::decode(instruction);
    s64 rdhilo = static_cast<s64>((static_cast<u64>(state.gpr[opcode.rd]) << 32) | (static_cast<u64>(state.gpr[opcode.rn])));
    s64 lhs;
    s64 rhs;

    if (opcode.x) {
        lhs = static_cast<s64>(static_cast<s16>(state.gpr[opcode.rm] >> 16));
    } else {
        lhs = static_cast<s64>(static_cast<s16>(state.gpr[opcode.rm]));
    }

    if (opcode.y) {
        rhs = static_cast<s64>(static_cast<s16>(state.gpr[opcode.rs] >> 16));
    } else {
        rhs = static_cast<s64>(static_cast<s16>(state.gpr[opcode.rs]));
    }

    s64 result = lhs * rhs;
    result += rdhilo;
    state.gpr[opcode.rn] = result & 0xffffffff;
    state.gpr[opcode.rd] = result >> 32;
    state.gpr[15] += 4;
}

void Interpreter::arm_branch_link_maybe_exchange() {
    if ((instruction & 0xf0000000) != 0xf0000000) {
        arm_branch_link();
    } else {
        arm_branch_link_exchange();
    }
}

void Interpreter::arm_branch_exchange() {
    auto opcode = ARMBranchExchange::decode(instruction);
    if (state.gpr[opcode.rm] & 0x1) {
        state.cpsr.t = true;
        state.gpr[15] = state.gpr[opcode.rm] & ~0x1;
        thumb_flush_pipeline();
    } else {
        state.gpr[15] = state.gpr[opcode.rm] & ~0x3;
        arm_flush_pipeline();
    }
}

void Interpreter::arm_branch_link() {
    auto opcode = ARMBranchLink::decode(instruction);
    if (opcode.link) {
        state.gpr[14] = state.gpr[15] - 4;
    }

    state.gpr[15] += opcode.offset;
    arm_flush_pipeline();
}

void Interpreter::arm_branch_link_exchange() {
    if (arch == Arch::ARMv4) {
        logger.warn("Interpreter: arm_branch_link_exchange executed by arm7");
        return;
    }

    auto opcode = ARMBranchLinkExchange::decode(instruction);
    state.gpr[14] = state.gpr[15] - 4;
    state.cpsr.t = true;
    state.gpr[15] += opcode.offset;
    thumb_flush_pipeline();
}

void Interpreter::arm_branch_link_exchange_register() {
    if (arch == Arch::ARMv4) {
        logger.warn("Interpreter: arm_branch_link_exchange_register executed by arm7");
        return;
    }

    auto opcode = ARMBranchExchange::decode(instruction);
    state.gpr[14] = state.gpr[15] - 4;
    if (state.gpr[opcode.rm] & 0x1) {
        state.cpsr.t = true;
        state.gpr[15] = state.gpr[opcode.rm] & ~0x1;
        thumb_flush_pipeline();
    } else {
        state.gpr[15] = state.gpr[opcode.rm] & ~0x3;
        arm_flush_pipeline();
    }
}

void Interpreter::arm_software_interrupt() {
    state.spsr_banked[Bank::SVC].data = state.cpsr.data;
    set_mode(Mode::SVC);

    state.cpsr.i = true;
    state.gpr[14] = state.gpr[15] - 4;
    state.gpr[15] = coprocessor.get_exception_base() + 0x08;
    arm_flush_pipeline();
}

void Interpreter::arm_breakpoint() {
    logger.todo("Interpreter: implement arm_breakpoint");
}

void Interpreter::arm_halfword_data_transfer() {
    auto opcode = ARMHalfwordDataTransfer::decode(instruction);
    
    if (opcode.rd == 15) {
        logger.error("Interpreter: handle rd == 15 in arm_halfword_data_transfer");
    }

    u32 op2 = 0;
    u32 addr = state.gpr[opcode.rn];
    bool do_writeback = !opcode.load || opcode.rd != opcode.rn;
    if (opcode.imm) {
        op2 = opcode.rhs.imm;
    } else {
        op2 = state.gpr[opcode.rhs.rm];
    }

    if (!opcode.up) {
        op2 *= -1;
    }

    if (opcode.pre) {
        addr += op2;
    }

    state.gpr[15] += 4;

    if (opcode.half && opcode.sign) {
        if (opcode.load) {
            state.gpr[opcode.rd] = common::sign_extend<s32, 16>(read_half(addr));
        } else if (arch == Arch::ARMv5) {
            if (opcode.rd & 0x1) {
                logger.error("Interpreter: undefined strd exception");
            }

            write_word(addr, state.gpr[opcode.rd]);
            write_word(addr + 4, state.gpr[opcode.rd + 1]);
        }
    } else if (opcode.half) {
        if (opcode.load) {
            state.gpr[opcode.rd] = read_half(addr);
        } else {
            write_half(addr, state.gpr[opcode.rd]);
        }
    } else if (opcode.sign) {
        if (opcode.load) {
            state.gpr[opcode.rd] = common::sign_extend<s32, 8>(read_byte(addr));
        } else if (arch == Arch::ARMv5) {
            if (opcode.rd & 0x1) {
                logger.error("Interpreter: undefined ldrd exception");
            }

            state.gpr[opcode.rd] = read_word(addr);
            state.gpr[opcode.rd + 1] = read_word(addr + 4);

            // when rn == rd + 1 writeback is not performed
            do_writeback = opcode.rn != (opcode.rd + 1);

            // when rd == 14 the pipeline is flushed
            // due to writing to r15
            if (opcode.rd == 14) {
                arm_flush_pipeline();
            }
        }
    }

    if (do_writeback) {
        if (!opcode.pre) {
            state.gpr[opcode.rn] += op2;
        } else if (opcode.writeback) {
            state.gpr[opcode.rn] = addr;
        }
    }
}

void Interpreter::arm_status_load() {
    auto opcode = ARMStatusLoad::decode(instruction);
    if (opcode.spsr) {
        state.gpr[opcode.rd] = state.spsr->data;
    } else {
        state.gpr[opcode.rd] = state.cpsr.data;
    }
    
    state.gpr[15] += 4;
}

void Interpreter::arm_status_store() {
    auto opcode = ARMStatusStore::decode(instruction);
    
    u32 value = 0;
    if (opcode.imm) {
        value = opcode.rhs.rotated;
    } else {
        value = state.gpr[opcode.rhs.rm];
    }

    if (opcode.spsr) {
        state.spsr->data = (state.spsr->data & ~opcode.mask) | (value & opcode.mask);
    } else {
        if (opcode.mask & 0xff) {
            set_mode(static_cast<Mode>(value & 0x1f));
        }

        state.cpsr.data = (state.cpsr.data & ~opcode.mask) | (value & opcode.mask);
    }

    state.gpr[15] += 4;
}

void Interpreter::arm_block_data_transfer() {
    auto opcode = ARMBlockDataTransfer::decode(instruction);
    u32 addr = state.gpr[opcode.rn];
    Mode old_mode = state.cpsr.mode;
    int first = 0;
    int bytes = 0;
    u32 new_base = 0;

    if (opcode.rlist != 0) {
        for (int i = 15; i >= 0; i--) {
            if (opcode.rlist & (1 << i)) {
                first = i;
                bytes += 4;
            }
        }
    } else {
        bytes = 0x40;
        if (arch == Arch::ARMv4) {
            opcode.rlist = 1 << 15;
            opcode.r15_in_rlist = true;
        }
    }

    if (opcode.up) {
        new_base = addr + bytes;
    } else {
        opcode.pre = !opcode.pre;
        addr -= bytes;
        new_base = addr;
    }

    state.gpr[15] += 4;

    // stm armv4: store old base if rb is first in rlist, otherwise store new base
    // stm armv5: always store old base
    if (opcode.writeback && !opcode.load) {
        if ((arch == Arch::ARMv4) && (first != opcode.rn)) {
            state.gpr[opcode.rn] = new_base;
        }
    }

    bool user_switch_mode = opcode.psr && (!opcode.load || !opcode.r15_in_rlist);
    if (user_switch_mode) {
        set_mode(Mode::USR);
    }

    for (int i = first; i < 16; i++) {
        if (!(opcode.rlist & (1 << i))) {
            continue;
        }

        if (opcode.pre) {
            addr += 4;
        }

        if (opcode.load) {
            state.gpr[i] = read_word(addr);
        } else {
            write_word(addr, state.gpr[i]);
        }

        if (!opcode.pre) {
            addr += 4;
        } 
    }

    if (opcode.writeback) {
        // ldm armv4: writeback if rn is not in rlist
        // ldm armv5: writeback if rn is only register or not the last register in rlist
        if (opcode.load) {
            if (arch == Arch::ARMv5) {
                if ((opcode.rlist == (1 << opcode.rn)) || !((opcode.rlist >> opcode.rn) == 1)) {
                    state.gpr[opcode.rn] = new_base;
                }
            } else {
                if (!(opcode.rlist & (1 << opcode.rn))) {
                    state.gpr[opcode.rn] = new_base;
                }
            }
        } else {
            state.gpr[opcode.rn] = new_base;
        }
    } 

    if (user_switch_mode) {
        set_mode(old_mode);
        if (opcode.load && opcode.r15_in_rlist) {
            logger.todo("Interpreter: handle loading into r15 in user mode");
        }
    }

    if (opcode.load && opcode.r15_in_rlist) {
        if ((arch == Arch::ARMv5) && (state.gpr[15] & 0x1)) {
            state.cpsr.t = true;
            thumb_flush_pipeline();
        } else {
            arm_flush_pipeline();
        }
    }
}

void Interpreter::arm_single_data_transfer() {
    auto opcode = ARMSingleDataTransfer::decode(instruction);
    u32 op2 = 0;
    u32 addr = state.gpr[opcode.rn];
    bool do_writeback = !opcode.load || opcode.rd != opcode.rn;

    if (opcode.imm) {
        op2 = opcode.rhs.imm;
    } else {
        bool carry = state.cpsr.c;
        op2 = barrel_shifter(state.gpr[opcode.rhs.reg.rm], opcode.rhs.reg.shift_type, opcode.rhs.reg.amount, carry, true);
    }

    if (!opcode.up) {
        op2 *= -1;
    }

    if (opcode.pre) {
        addr += op2;
    }

    state.gpr[15] += 4;

    if (opcode.load) {
        if (opcode.byte) {
            state.gpr[opcode.rd] = read_byte(addr);
        } else {
            state.gpr[opcode.rd] = read_word_rotate(addr);
        }
    } else {
        if (opcode.byte) {
            write_byte(addr, state.gpr[opcode.rd]);
        } else {
            write_word(addr, state.gpr[opcode.rd]);
        }
    }

    if (do_writeback) {
        if (!opcode.pre) {
            state.gpr[opcode.rn] += op2;
        } else if (opcode.writeback) {
            state.gpr[opcode.rn] = addr;
        }
    }

    if (opcode.load && opcode.rd == 15) {
        if ((arch == Arch::ARMv5) && (state.gpr[15] & 0x1)) {
            state.cpsr.t = true;
            state.gpr[15] &= ~0x1;
            thumb_flush_pipeline();
        } else {
            state.gpr[15] &= ~0x3;
            arm_flush_pipeline();
        }
    }
}

void Interpreter::arm_coprocessor_register_transfer() {
    auto opcode = ARMCoprocessorRegisterTransfer::decode(instruction);

    // TODO: handle this in a nicer way
    if (arch == Arch::ARMv4 && opcode.cp == 14) {
        logger.warn("Interpreter: mrc cp14 on arm7");
        return;
    } else if ((arch == Arch::ARMv4 && opcode.cp == 15) || (arch == Arch::ARMv5 && opcode.cp == 14)) {
        undefined_exception();
        return;
    }

    if (opcode.rd == 15) {
        logger.error("Interpreter: handle rd == 15 in arm_coprocessor_register_transfer");
    }

    if (opcode.load) {
        state.gpr[opcode.rd] = coprocessor.read(opcode.crn, opcode.crm, opcode.cp);
    } else {
        coprocessor.write(opcode.crn, opcode.crm, opcode.cp, state.gpr[opcode.rd]);
    }

    state.gpr[15] += 4;
}

} // namespace arm