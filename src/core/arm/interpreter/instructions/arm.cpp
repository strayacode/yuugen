#include "common/bits.h"
#include "common/logger.h"
#include "core/arm/interpreter/interpreter.h"

namespace core::arm {

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
            // // store the current spsr in cpsr only if in privileged mode
            // if (is_privileged()) {
            //     u32 current_spsr = get_spsr();
                
            //     switch_mode(current_spsr & 0x1F);
            //     state.cpsr.data = current_spsr;
            // } else {
            //     logger.error("[ARM] Loading spsr into cpsr in non-privileged mode is undefined behaviour");
            // }

            // if (is_arm()) {
            //     arm_flush_pipeline();
            // } else {
            //     thumb_flush_pipeline();
            // }
            logger.error("Interpreter: handle rd == 15 and S == 1");
        } else {
            arm_flush_pipeline();
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
        return;
    }

    auto opcode = ARMCountLeadingZeroes::decode(instruction);
    state.gpr[opcode.rd] = common::countl_zeroes(state.gpr[opcode.rm]);
    state.gpr[15] += 4;
}

void Interpreter::arm_saturating_add_subtract() {
    if (arch == Arch::ARMv4) {
        return;
    }

    auto opcode = ARMSaturatingAddSubtract::decode(instruction);
    u32 lhs = state.gpr[opcode.rm];
    u32 rhs = state.gpr[opcode.rn];

    if (opcode.rd == 15) {
        logger.todo("Interpreter: handle rd == 15 in arm_saturating_add_subtract");
    }

    if (opcode.double_rhs) {
        u32 result = rhs * 2;
        if ((rhs ^ result) >> 31) {
            state.cpsr.q = true;
            result = 0x80000000 - (result >> 31);
        }

        rhs = result;
    }

    if (opcode.sub) {
        u32 result = lhs - rhs;
        if (calculate_add_overflow(lhs, rhs, result)) {
            state.cpsr.q = true;
            result = 0x80000000 - (result >> 31);
        }

        state.gpr[opcode.rd] = result;
    } else {
        u32 result = lhs + rhs;
        if (calculate_sub_overflow(lhs, rhs, result)) {
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

    u8 op1 = instruction & 0xF;
    u8 op2 = (instruction >> 8) & 0xF;
    u8 op3 = (instruction >> 12) & 0xF;
    u8 op4 = (instruction >> 16) & 0xF;

    bool x = instruction & (1 << 5);
    bool y = instruction & (1 << 6);

    s64 rdhilo = (s64)(((u64)state.gpr[op4] << 32) | ((u64)state.gpr[op3]));

    s64 result1;
    s64 result2;

    if (x) {
        result1 = (s64)(s16)(state.gpr[op1] >> 16);
    } else {
        result1 = (s64)(s16)state.gpr[op1];
    }

    if (y) {
        result2 = (s64)(s16)(state.gpr[op2] >> 16);
    } else {
        result2 = (s64)(s16)state.gpr[op2];
    }

    s64 result = result1 * result2;
    result += rdhilo;
    state.gpr[op3] = result;
    state.gpr[op4] = result >> 32;

    state.gpr[15] += 4;
}

void Interpreter::arm_branch_link_maybe_exchange() {
    // if ((instruction & 0xF0000000) != 0xF0000000) {
    //     arm_branch_link();
    // } else {
    //     arm_branch_link_exchange();
    // }
}

void Interpreter::arm_branch_exchange() {
    // u8 rm = instruction & 0xF;
    // if (state.gpr[rm] & 0x1) {
    //     // switch to thumb mode execution
    //     state.cpsr.t = true;
    //     state.gpr[15] = state.gpr[rm] & ~1;
    //     thumb_flush_pipeline();
    // } else {
    //     state.gpr[15] = state.gpr[rm] & ~3;
    //     arm_flush_pipeline();
    // }
}

void Interpreter::arm_branch_link() {
    // const bool link = (instruction >> 24) & 0x1;
    // u32 offset = ((instruction & (1 << 23)) ? 0xFC000000 : 0) | ((instruction & 0xFFFFFF) << 2);
    
    // if (link) {
    //     // store the address of the instruction after the current instruction in the link register
    //     state.gpr[14] = state.gpr[15] - 4;
    // }
    
    // // r15 is at instruction address + 8
    // state.gpr[15] += offset;

    // arm_flush_pipeline();
}

void Interpreter::arm_branch_link_exchange() {
    // if (arch == Arch::ARMv4) {
    //     return;
    // }

    // state.gpr[14] = state.gpr[15] - 4;
    // state.cpsr.t = true;

    // u32 offset = (((instruction & (1 << 23)) ? 0xFC000000: 0) | ((instruction & 0xFFFFFF) << 2)) + ((instruction & (1 << 24)) >> 23);
    // state.gpr[15] += offset;
    // thumb_flush_pipeline();
}

void Interpreter::arm_branch_link_exchange_register() {
    // if (arch == Arch::ARMv4) {
    //     return;
    // }

    // state.gpr[14] = state.gpr[15] - 4;

    // u8 rm = instruction & 0xF;
    // if (state.gpr[rm] & 0x1) {
    //     state.cpsr.t = true;
    //     state.gpr[15] = state.gpr[rm] & ~1;
    //     thumb_flush_pipeline();
    // } else {
    //     state.gpr[15] = state.gpr[rm] & ~3;
    //     arm_flush_pipeline();
    // }
}

void Interpreter::arm_software_interrupt() {
    // state.spsr_banked[BANK_SVC].data = state.cpsr.data;

    // switch_mode(MODE_SVC);

    // // disable interrupts
    // state.cpsr.i = true;
    // state.gpr[14] = state.gpr[15] - 4;

    // // jump to the exception base in the bios
    // state.gpr[15] = coprocessor.get_exception_base() + 0x08;
    // arm_flush_pipeline();
}

void Interpreter::arm_breakpoint() {
    logger.todo("Interpreter: implement arm_breakpoint");
}

void Interpreter::arm_halfword_data_transfer() {
    // const bool load = (m_instruction >> 20) & 0x1;
    // const bool writeback = (m_instruction >> 21) & 0x1;
    // const bool immediate = (m_instruction >> 22) & 0x1;
    // const bool up = (m_instruction >> 23) & 0x1;
    // const bool pre = (m_instruction >> 24) & 0x1;
    // u8 rm = m_instruction & 0xF;
    // u8 opcode = (m_instruction >> 5) & 0x3;
    // u8 rd = (m_instruction >> 12) & 0xF;
    // u8 rn = (m_instruction >> 16) & 0xF;

    // u32 op2 = 0;
    // u32 address = m_gpr[rn];

    // bool do_writeback = !load || rd != rn;

    // if (immediate) {
    //     op2 = ((m_instruction >> 4) & 0xF0) | (m_instruction & 0xF);
    // } else {
    //     op2 = m_gpr[rm];
    // }

    // if (!up) {
    //     op2 *= -1;
    // }

    // if (pre) {
    //     address += op2;
    // }

    // m_gpr[15] += 4;

    // switch (opcode) {
    // case 0x1:
    //     if (load) {
    //         m_gpr[rd] = read_half(address);
    //     } else {
    //         write_half(address, m_gpr[rd]);
    //     }
    //     break;
    // case 0x2:
    //     if (load) {
    //         m_gpr[rd] = Common::sign_extend<u32, 8>(read_byte(address));
    //     } else if (m_arch == Arch::ARMv5) {
    //         // cpu locks up when rd is odd
    //         if (rd & 0x1) {
    //             logger.error("undefined ldrd exception");
    //         }

    //         m_gpr[rd] = read_word(address);
    //         m_gpr[rd + 1] = read_word(address + 4);

    //         // when rn == rd + 1 writeback is not performed
    //         do_writeback = rn != (rd + 1);

    //         // when rd == 14 the pipeline is flushed
    //         // due to writing to r15
    //         if (rd == 14) {
    //             arm_flush_pipeline();
    //         }
    //     }
    //     break;
    // case 0x3:
    //     if (load) {
    //         m_gpr[rd] = Common::sign_extend<u32, 16>(read_half(address));
    //     } else if (m_arch == Arch::ARMv5) {
    //         // cpu locks up when rd is odd
    //         if (rd & 0x1) {
    //             logger.error("undefined strd exception");
    //         }

    //         write_word(address, m_gpr[rd]);
    //         write_word(address + 4, m_gpr[rd + 1]);
    //     }
    //     break;
    // default:
    //     logger.error("handle opcode %d", opcode);
    // }

    // if (do_writeback) {
    //     if (!pre) {
    //         m_gpr[rn] += op2;
    //     } else if (writeback) {
    //         m_gpr[rn] = address;
    //     }
    // }

    // if (rd == 15) {
    //     logger.error("handle");
    // }
}

void Interpreter::arm_psr_transfer() {
    // const bool opcode = (m_instruction >> 21) & 0x1;
    // const bool spsr = (m_instruction >> 22) & 0x1;
    // u8 rm = m_instruction & 0xF;

    // if (opcode) {
    //     // msr
    //     u8 immediate = (m_instruction >> 25) & 0x1;
    //     u32 value = 0;

    //     u32 mask = 0;
    //     if (m_instruction & (1 << 16)) {
    //         mask |= 0x000000FF;
    //     }
    //     if (m_instruction & (1 << 17)) {
    //         mask |= 0x0000FF00;
    //     }
    //     if (m_instruction & (1 << 18)) {
    //         mask |= 0x00FF0000;
    //     }
    //     if (m_instruction & (1 << 19)) {
    //         mask |= 0xFF000000;
    //     }

    //     if (immediate) {
    //         u32 immediate = m_instruction & 0xFF;
    //         u8 rotate_amount = ((m_instruction >> 8) & 0xF) << 1;

    //         value = Common::rotate_right(immediate, rotate_amount);
    //     } else {
    //         value = m_gpr[rm];
    //     }

    //     // TODO: check later
    //     if (spsr) {
    //         if (has_spsr()) {
    //             set_spsr((get_spsr() & ~mask) | (value & mask));
    //         }
    //     } else {
    //         if (m_instruction & (1 << 16) && is_privileged()) {
    //             switch_mode(value & 0x1F);
    //         }

    //         m_cpsr.data = (m_cpsr.data & ~mask) | (value & mask);
    //     }
    // } else {
    //     // mrs
    //     u8 rd = (m_instruction >> 12) & 0xF;

    //     if (spsr) {
    //         m_gpr[rd] = get_spsr();
    //     } else {
    //         m_gpr[rd] = m_cpsr.data;
    //     }
    // }
    
    // m_gpr[15] += 4;
}

void Interpreter::arm_block_data_transfer() {
    // const bool load = (m_instruction >> 20) & 0x1;
    // const bool writeback = (m_instruction >> 21) & 0x1;
    // const bool load_psr = (m_instruction >> 22) & 0x1;
    // const bool up = (m_instruction >> 23) & 0x1;
    // bool pre = (m_instruction >> 24) & 0x1;
    // u8 rn = (m_instruction >> 16) & 0xF;
    // u8 r15_in_rlist = (m_instruction >> 15) & 0x1;
    // u32 address = m_gpr[rn];
    // u8 old_mode = m_cpsr.mode;
    // u16 rlist = m_instruction & 0xFFFF;
    // int first = 0;
    // int bytes = 0;
    // u32 new_base = 0;

    // if (rlist != 0) {
    //     for (int i = 15; i >= 0; i--) {
    //         if (rlist & (1 << i)) {
    //             first = i;
    //             bytes += 4;
    //         }
    //     }
    // } else {
    //     // handle empty rlist
    //     bytes = 0x40;

    //     if (m_arch == Arch::ARMv4) {
    //         // only r15 gets transferred
    //         rlist = 1 << 15;
    //         r15_in_rlist = true;
    //     }
    // }

    // if (up) {
    //     new_base = address + bytes;
    // } else {
    //     pre = !pre;
    //     address -= bytes;
    //     new_base = address;
    // }

    // // increment r15 before doing transfer because if r15 in rlist and stm is used,
    // // the value written is the address of the stm instruction + 12
    // m_gpr[15] += 4;

    // // stm armv4: store old base if rb is first in rlist, otherwise store new base
    // // stm armv5: always store old base
    // if (writeback && !load) {
    //     if ((m_arch == Arch::ARMv4) && (first != rn)) {
    //         m_gpr[rn] = new_base;
    //     }
    // }

    // // make sure to only do user bank transfer if r15 is not in rlist or instruction is not ldm
    // // ~(A and B) = ~A or ~B
    // bool user_switch_mode = load_psr && (!load || !r15_in_rlist);

    // if (user_switch_mode) {
    //     switch_mode(MODE_USR);
    // }

    // // registers are transferred in order from lowest to highest
    // for (int i = first; i < 16; i++) {
    //     if (!(rlist & (1 << i))) {
    //         continue;
    //     }

    //     if (pre) {
    //         address += 4;
    //     }

    //     if (load) {
    //         m_gpr[i] = read_word(address);
    //     } else {
    //         write_word(address, m_gpr[i]);
    //     }

    //     if (!pre) {
    //         address += 4;
    //     } 
    // }

    // if (writeback) {
    //     // ldm armv4: writeback if rb is not in rlist
    //     // ldm armv5: writeback if rb is only register or not the last register in rlist
    //     if (load) {
    //         if (m_arch == Arch::ARMv5) {
    //             if ((rlist == (1 << rn)) || !((rlist >> rn) == 1)) {
    //                 m_gpr[rn] = new_base;
    //             }
    //         } else {
    //             if (!(rlist & (1 << rn))) {
    //                 m_gpr[rn] = new_base;
    //             }
    //         }
    //     } else {
    //         m_gpr[rn] = new_base;
    //     }
    // } 

    // if (user_switch_mode) {
    //     // switch back to old mode at the end if user bank transfer
    //     switch_mode(old_mode);

    //     if (load && r15_in_rlist) {
    //         todo();
    //     }
    // }

    // if (load && r15_in_rlist) {
    //     if ((m_arch == Arch::ARMv5) && (m_gpr[15] & 0x1)) {
    //         m_cpsr.t = true;
    //         thumb_flush_pipeline();
    //     } else {
    //         arm_flush_pipeline();
    //     }
    // }
}

void Interpreter::arm_single_data_transfer() {
    // const bool load = (m_instruction >> 20) & 0x1;
    // const bool writeback = (m_instruction >> 21) & 0x1;
    // const bool byte = (m_instruction >> 22) & 0x1;
    // const bool up = (m_instruction >> 23) & 0x1;
    // const bool pre = (m_instruction >> 24) & 0x1;
    // const bool shifted_register = (m_instruction >> 25) & 0x1;
    // u8 rd = (m_instruction >> 12) & 0xF;
    // u8 rn = (m_instruction >> 16) & 0xF;

    // u32 op2 = 0;
    // u32 address = m_gpr[rn];

    // if (shifted_register) {
    //     op2 = arm_get_shifted_register_single_data_transfer();
    // } else {
    //     op2 = m_instruction & 0xFFF;
    // }

    // if (!up) {
    //     op2 *= -1;
    // }

    // if (pre) {
    //     address += op2;
    // }

    // // increment r15 by 4 since if we are writing r15 it must be r15 + 12.
    // // however if we are loading into r15 this increment won't matter as well
    // m_gpr[15] += 4;

    // if (load) {
    //     if (byte) {
    //         m_gpr[rd] = read_byte(address);
    //     } else {
    //         m_gpr[rd] = read_word_rotate(address);
    //     }
    // } else {
    //     if (byte) {
    //         write_byte(address, m_gpr[rd]);
    //     } else {
    //         write_word(address, m_gpr[rd]);
    //     }
    // }

    // if (!load || rd != rn) {
    //     if (!pre) {
    //         m_gpr[rn] += op2;
    //     } else if (writeback) {
    //         m_gpr[rn] = address;
    //     }
    // }

    // if (load && rd == 15) {
    //     if ((m_arch == Arch::ARMv5) && (m_gpr[15] & 1)) {
    //         m_cpsr.t = true;
    //         m_gpr[15] &= ~1;
    //         thumb_flush_pipeline();
    //     } else {
    //         m_gpr[15] &= ~3;
    //         arm_flush_pipeline();
    //     }
    // }
}

void Interpreter::arm_coprocessor_register_transfer() {
    // if (m_arch == Arch::ARMv4) {
    //     return;
    // }

    // u8 crm = m_instruction & 0xF;
    // u8 crn = (m_instruction >> 16) & 0xF;
    // u8 opcode2 = (m_instruction >> 5) & 0x7;
    // u8 rd = (m_instruction >> 12) & 0xF;

    // if (m_instruction & (1 << 20)) {
    //     m_gpr[rd] = m_coprocessor.read(crn, crm, opcode2);

    //     if (rd == 15) {
    //         todo();
    //     }
    // } else {
    //     m_coprocessor.write(crn, crm, opcode2, m_gpr[rd]);
    // }

    // m_gpr[15] += 4;
}

} // namespace core::arm