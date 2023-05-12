#include "common/logger.h"
#include "common/bits.h"
#include "arm/interpreter/interpreter.h"

namespace arm {

void Interpreter::thumb_add_subtract() {
    auto opcode = ThumbAddSubtract::decode(instruction);
    u32 lhs = state.gpr[opcode.rs];
    u32 rhs = opcode.imm ? opcode.rn : state.gpr[opcode.rn];

    if (opcode.sub) {
        state.gpr[opcode.rd] = alu_sub(lhs, rhs, true);
    } else {
        state.gpr[opcode.rd] = alu_add(lhs, rhs, true);
    }

    state.gpr[15] += 2;
}

void Interpreter::thumb_shift_immediate() {
    auto opcode = ThumbShiftImmediate::decode(instruction);
    bool carry = state.cpsr.c;

    state.gpr[opcode.rd] = barrel_shifter(state.gpr[opcode.rs], opcode.shift_type, opcode.amount, carry, true);
    state.cpsr.c = carry;
    set_nz(state.gpr[opcode.rd]);
    state.gpr[15] += 2;
}

void Interpreter::thumb_alu_immediate() {
    auto opcode = ThumbALUImmediate::decode(instruction);
    switch (opcode.opcode) {
    case ThumbALUImmediate::MOV:
        state.gpr[opcode.rd] = opcode.imm;
        state.cpsr.n = false;
        state.cpsr.z = opcode.imm == 0;
        break;
    case 0x1:
        alu_cmp(state.gpr[opcode.rd], opcode.imm);
        break;
    case 0x2:
        state.gpr[opcode.rd] = alu_add(state.gpr[opcode.rd], opcode.imm, true);
        break;
    case 0x3:
        state.gpr[opcode.rd] = alu_sub(state.gpr[opcode.rd], opcode.imm, true);
        break;
    }

    state.gpr[15] += 2;
}

void Interpreter::thumb_data_processing_register() {
    logger.todo("handle thumb_data_processing_register");
    // u8 rd = instruction & 0x7;
    // u8 rs = (instruction >> 3) & 0x7;
    // u8 opcode = (instruction >> 6) & 0xF;
    // bool carry = state.cpsr.c;

    // switch (opcode) {
    // case 0x0:
    //     state.gpr[rd] = alu_and(state.gpr[rd], state.gpr[rs], true);
    //     break;
    // case 0x1:
    //     state.gpr[rd] = alu_eor(state.gpr[rd], state.gpr[rs], true);
    //     break;
    // case 0x2:
    //     state.gpr[rd] = alu_lsl(state.gpr[rd], state.gpr[rs] & 0xFF, carry);
    //     state.cpsr.c = carry;
    //     state.cpsr.n = state.gpr[rd] >> 31;
    //     state.cpsr.z = state.gpr[rd] == 0;
    //     break;
    // case 0x3:
    //     state.gpr[rd] = alu_lsr(state.gpr[rd], state.gpr[rs] & 0xFF, carry, false);
    //     state.cpsr.c = carry;
    //     state.cpsr.n = state.gpr[rd] >> 31;
    //     state.cpsr.z = state.gpr[rd] == 0;
    //     break;
    // case 0x4:
    //     state.gpr[rd] = alu_asr(state.gpr[rd], state.gpr[rs] & 0xFF, carry, false);
    //     state.cpsr.c = carry;
    //     state.cpsr.n = state.gpr[rd] >> 31;
    //     state.cpsr.z = state.gpr[rd] == 0;
    //     break;
    // case 0x5:
    //     state.gpr[rd] = alu_adc(state.gpr[rd], state.gpr[rs], true);
    //     break;
    // case 0x6:
    //     state.gpr[rd] = alu_sbc(state.gpr[rd], state.gpr[rs], true);
    //     break;
    // case 0x7:
    //     state.gpr[rd] = alu_ror(state.gpr[rd], state.gpr[rs] & 0xFF, carry, false);
    //     state.cpsr.c = carry;
    //     state.cpsr.n = state.gpr[rd] >> 31;
    //     state.cpsr.z = state.gpr[rd] == 0;
    //     break;
    // case 0x8:
    //     alu_tst(state.gpr[rd], state.gpr[rs]);
    //     break;
    // case 0x9:
    //     state.gpr[rd] = alu_sub(0, state.gpr[rs], true);
    //     break;
    // case 0xA:
    //     alu_cmp(state.gpr[rd], state.gpr[rs]);
    //     break;
    // case 0xB:
    //     alu_cmn(state.gpr[rd], state.gpr[rs]);
    //     break;
    // case 0xC:
    //     state.gpr[rd] = alu_orr(state.gpr[rd], state.gpr[rs], true);
    //     break;
    // case 0xD:
    //     state.gpr[rd] *= state.gpr[rs];

    //     state.cpsr.n = state.gpr[rd] >> 31;
    //     state.cpsr.z = state.gpr[rd] == 0;
    //     break;
    // case 0xE:
    //     state.gpr[rd] = alu_bic(state.gpr[rd], state.gpr[rs], true);
    //     break;
    // case 0xF:
    //     state.gpr[rd] = alu_mvn(state.gpr[rs], true);
    //     break;
    // }

    // state.gpr[15] += 2;
}

void Interpreter::thumb_special_data_processing() {
    auto opcode = ThumbSpecialDataProcessing::decode(instruction);
    switch (opcode.opcode) {
    case ThumbSpecialDataProcessing::Opcode::ADD:
        state.gpr[opcode.rd] += state.gpr[opcode.rs];
        if (opcode.rd == 15) {
            thumb_flush_pipeline();
        } else {
            state.gpr[15] += 2;    
        }

        break;
    case ThumbSpecialDataProcessing::Opcode::CMP:
        alu_cmp(state.gpr[opcode.rd], state.gpr[opcode.rs]);
        state.gpr[15] += 2;
        break;
    case ThumbSpecialDataProcessing::Opcode::MOV:
        state.gpr[opcode.rd] = state.gpr[opcode.rs];
        if (opcode.rd == 15) {
            thumb_flush_pipeline();
        } else {
            state.gpr[15] += 2;
        }

        break;
    }
}

void Interpreter::thumb_adjust_stack_pointer() {
    auto opcode = ThumbAdjustStackPointer::decode(instruction);
    if (opcode.sub) {
        state.gpr[13] -= opcode.imm;
    } else {
        state.gpr[13] += opcode.imm;
    }
    
    state.gpr[15] += 2;
}

void Interpreter::thumb_add_sp_pc() {
    auto opcode = ThumbAddSPPC::decode(instruction);
    if (opcode.sp) {
        state.gpr[opcode.rd] = state.gpr[13] + opcode.imm;
    } else {
        state.gpr[opcode.rd] = (state.gpr[15] & ~0x2) + opcode.imm;
    }

    state.gpr[15] += 2;
}

void Interpreter::thumb_branch_exchange() {
    auto opcode = ThumbBranchExchange::decode(instruction);
    if (state.gpr[opcode.rm] & 0x1) {
        state.gpr[15] = state.gpr[opcode.rm] & ~0x1;
        thumb_flush_pipeline();
    } else {
        state.cpsr.t = false;
        state.gpr[15] = state.gpr[opcode.rm] & ~0x3;
        arm_flush_pipeline();
    }
}

void Interpreter::thumb_branch_link_exchange() {
    logger.todo("handle thumb_branch_link_exchange");
    // if (arch == Arch::ARMv4) {
    //     return;
    // }

    // u8 rm = (instruction >> 3) & 0xF;
    // u32 next_instruction_address = state.gpr[15] - 2;
    // state.gpr[14] = next_instruction_address | 1;
    
    // if (state.gpr[rm] & 0x1) {
    //     state.gpr[15] = state.gpr[rm] & ~1;
    //     thumb_flush_pipeline();
    // } else {
    //     state.cpsr.t = false;
    //     state.gpr[15] = state.gpr[rm] & ~3;
    //     arm_flush_pipeline();
    // }
}

void Interpreter::thumb_branch_link_setup() {
    auto opcode = ThumbBranchLinkSetup::decode(instruction);
    state.gpr[14] = state.gpr[15] + opcode.imm;
    state.gpr[15] += 2;
}

void Interpreter::thumb_branch_link_offset() {
    auto opcode = ThumbBranchLinkOffset::decode(instruction); 
    u32 next_instruction_addr = state.gpr[15] - 2;
    state.gpr[15] = (state.gpr[14] + opcode.offset) & ~0x1;
    state.gpr[14] = next_instruction_addr | 0x1;
    thumb_flush_pipeline();
}

void Interpreter::thumb_branch_link_exchange_offset() {
    logger.todo("handle thumb_branch_link_exchange_offset");
    // // arm9 specific instruction
    // if (arch == Arch::ARMv4) {
    //     return;
    // }

    // u32 offset = (instruction & 0x7FF) << 1;
    // u32 next_instruction_address = state.gpr[15] - 2;
    // state.gpr[15] = (state.gpr[14] + offset) & ~0x3;
    // state.gpr[14] = next_instruction_address | 1;

    // // set t flag to 0
    // state.cpsr.t = false;

    // // flush the pipeline
    // arm_flush_pipeline();
}

void Interpreter::thumb_branch() {
    auto opcode = ThumbBranch::decode(instruction);
    state.gpr[15] += opcode.offset;
    thumb_flush_pipeline();
}

void Interpreter::thumb_branch_conditional() {
    auto opcode = ThumbBranchConditional::decode(instruction);
    if (evaluate_condition(opcode.condition)) {
        state.gpr[15] += opcode.offset;
        thumb_flush_pipeline();
    } else {
        state.gpr[15] += 2;
    }
}

void Interpreter::thumb_software_interrupt() {
    state.spsr_banked[Bank::SVC].data = state.cpsr.data;
    set_mode(Mode::SVC);

    state.cpsr.t = false;
    state.cpsr.i = true;
    state.gpr[14] = state.gpr[15] - 2;
    state.gpr[15] = coprocessor.get_exception_base() + 0x08;
    arm_flush_pipeline();
}

void Interpreter::thumb_load_pc() {
    auto opcode = ThumbLoadPC::decode(instruction);
    u32 addr = (state.gpr[15] & ~0x2) + opcode.imm;
    state.gpr[opcode.rd] = read_word(addr);
    state.gpr[15] += 2;
}

void Interpreter::thumb_load_store() {
    logger.todo("handle thumb_load_store");
    // u8 rd = m_instruction & 0x7;
    // u8 rn = (m_instruction >> 3) & 0x7;
    // u8 rm = (m_instruction >> 6) & 0x7;

    // u8 opcode = (m_instruction >> 10) & 0x3;
    // bool sign = m_instruction & (1 << 9);
    // u32 address = m_gpr[rn] + m_gpr[rm];

    // if (sign) {
    //     switch (opcode) {
    //     case 0x0:
    //         write_half(address, m_gpr[rd]);
    //         break;
    //     case 0x1:
    //         m_gpr[rd] = Common::sign_extend<u32, 8>(read_byte(address));
    //         break;
    //     case 0x2:
    //         m_gpr[rd] = read_half(address);
    //         break;
    //     case 0x3:
    //         m_gpr[rd] = Common::sign_extend<u32, 16>(read_half(address));
    //         break;
    //     }    
    // } else {
    //     switch (opcode) {
    //     case 0x0:
    //         write_word(address, m_gpr[rd]);
    //         break;
    //     case 0x1:
    //         write_byte(address, m_gpr[rd]);
    //         break;
    //     case 0x2: {
    //         m_gpr[rd] = read_word(address);

    //         if (address & 0x3) {
    //             int shift_amount = (address & 0x3) * 8;
    //             m_gpr[rd] = (m_gpr[rd] << (32 - shift_amount)) | (m_gpr[rd] >> shift_amount);
    //         }

    //         break;
    //     }
    //     case 0x3:
    //         m_gpr[rd] = read_byte(address);
    //         break;
    //     }
    // }
    
    // m_gpr[15] += 2;
}

void Interpreter::thumb_load_store_immediate() {
    logger.todo("handle thumb_load_store_immediate");
    // u8 rd = m_instruction & 0x7;
    // u8 rn = (m_instruction >> 3) & 0x7;
    // u32 immediate = (m_instruction >> 6) & 0x1F;

    // u8 opcode = (m_instruction >> 11) & 0x3;

    // switch (opcode) {
    // case 0x0:
    //     write_word(m_gpr[rn] + (immediate << 2), m_gpr[rd]);
    //     break;
    // case 0x1:
    //     m_gpr[rd] = read_word_rotate(m_gpr[rn] + (immediate << 2));
    //     break;
    // case 0x2:
    //     write_byte(m_gpr[rn] + immediate, m_gpr[rd]);
    //     break;
    // case 0x3:
    //     m_gpr[rd] = read_byte(m_gpr[rn] + immediate);
    //     break;
    // }
    
    // m_gpr[15] += 2;
}

void Interpreter::thumb_push_pop() {
    auto opcode = ThumbPushPop::decode(instruction);
    u32 addr = state.gpr[13];

    if (opcode.pop) {
        for (int i = 0; i < 8; i++) {
            if (opcode.rlist & (1 << i)) {
                state.gpr[i] = read_word(addr);
                addr += 4;
            }
        }

        if (opcode.pclr) {
            state.gpr[15] = read_word(addr);
            state.gpr[13] = addr + 4;
            
            if ((arch == Arch::ARMv4) || (state.gpr[15] & 0x1)) {
                state.gpr[15] &= ~0x1;
                thumb_flush_pipeline();
            } else {
                state.cpsr.t = false;
                state.gpr[15] &= ~0x3;
                arm_flush_pipeline();
            }
        } else {
            state.gpr[15] += 2;
            state.gpr[13] = addr;
        }
    } else {
        for (int i = 0; i < 8; i++) {
            if (opcode.rlist & (1 << i)) {
                addr -= 4;
            }
        }

        if (opcode.pclr) {
            addr -= 4;
        }

        state.gpr[13] = addr;

        for (int i = 0; i < 8; i++) {
            if (opcode.rlist & (1 << i)) {
                write_word(addr, state.gpr[i]);
                addr += 4;
            }
        }

        if (opcode.pclr) {
            write_word(addr, state.gpr[14]);
        }

        state.gpr[15] += 2;
    }
}

void Interpreter::thumb_load_store_sp_relative() {
    logger.todo("handle thumb_load_store_sp_relative");
    // u32 immediate = m_instruction & 0xFF;
    // u8 rd = (m_instruction >> 8) & 0x7;

    // bool load = m_instruction & (1 << 11);

    // u32 address = m_gpr[13] + (immediate << 2);

    // if (load) {
    //     m_gpr[rd] = read_word_rotate(address);
    // } else {
    //     write_word(address, m_gpr[rd]);
    // }
    
    // m_gpr[15] += 2;
}

void Interpreter::thumb_load_store_halfword() {
    auto opcode = ThumbLoadStoreHalfword::decode(instruction);
    u32 addr = state.gpr[opcode.rn] + (opcode.imm << 1);
    if (opcode.load) {
        state.gpr[opcode.rd] = read_half(addr);
    } else {
        write_half(addr, state.gpr[opcode.rd]);
    }

    state.gpr[15] += 2;
}

void Interpreter::thumb_load_store_multiple() {
    logger.todo("handle thumb_load_store_multiple");
    // u8 rn = (m_instruction >> 8) & 0x7;
    // u32 address = m_gpr[rn];

    // bool load = m_instruction & (1 << 11);
    
    // // TODO: handle edgecases
    // if (load) {
    //     for (int i = 0; i < 8; i++) {
    //         if (m_instruction & (1 << i)) {
    //             m_gpr[i] = read_word(address);
    //             address += 4;
    //         }
    //     }

    //     // if rn is in rlist:
    //     // if arm9 writeback if rn is the only register or not the last register in rlist
    //     // if arm7 then no writeback if rn in rlist
    //     if (m_arch == Arch::ARMv5) {
    //         if (((m_instruction & 0xFF) == static_cast<u32>(1 << rn)) || !(((m_instruction & 0xFF) >> rn) == 1)) {
    //             m_gpr[rn] = address;
    //         }
    //     } else if (!(m_instruction & static_cast<u32>(1 << rn))) {
    //         m_gpr[rn] = address;
    //     }
    // } else {
    //     for (int i = 0; i < 8; i++) {
    //         if (m_instruction & (1 << i)) {
    //             write_word(address, m_gpr[i]);
    //             address += 4;
    //         }
    //     }

    //     m_gpr[rn] = address;
    // }

    // m_gpr[15] += 2;
}

} // namespace arm