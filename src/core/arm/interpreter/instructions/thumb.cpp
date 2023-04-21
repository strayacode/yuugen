#include "common/logger.h"
#include "common/bits.h"
#include "core/arm/interpreter/interpreter.h"

namespace core::arm {

void Interpreter::thumb_add_subtract() {
    u8 rn = (instruction >> 6) & 0x7;
    u8 rs = (instruction >> 3) & 0x7;
    u8 rd = instruction & 0x7;

    bool immediate = instruction & (1 << 10);
    bool sub = instruction & (1 << 9);

    u32 op1 = state.gpr[rs];
    u32 op2 = immediate ? rn : state.gpr[rn];

    if (sub) {
        state.gpr[rd] = alu_sub(op1, op2, true);
    } else {
        state.gpr[rd] = alu_add(op1, op2, true);
    }

    state.gpr[15] += 2;
}

void Interpreter::thumb_shift_immediate() {
    u8 rd = instruction & 0x7;
    u8 rs = (instruction >> 3) & 0x7;

    u8 shift_amount = (instruction >> 6) & 0x1F;
    u8 shift_type = (instruction >> 11) & 0x3;

    u8 carry = state.cpsr.c;

    switch (shift_type) {
    case 0x0:
        if (shift_amount != 0) {
            carry = (state.gpr[rs] >> (32 - shift_amount)) & 0x1;
        }

        state.gpr[rd] = state.gpr[rs] << shift_amount;
        break;
    case 0x1:
        if (shift_amount == 0) {
            carry = state.gpr[rs] >> 31;
            state.gpr[rd] = 0;
        } else {
            carry = (state.gpr[rs] >> (shift_amount - 1)) & 0x1;
            state.gpr[rd] = state.gpr[rs] >> shift_amount;
        } 
        break;
    case 0x2: {
        u32 msb = state.gpr[rs] >> 31;

        if (shift_amount == 0) {
            carry = state.gpr[rd] >> 31;
            state.gpr[rd] = 0xFFFFFFFF * msb;
        } else {
            carry = (state.gpr[rs] >> (shift_amount - 1)) & 0x1;
            state.gpr[rd] = (state.gpr[rs] >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
        }
        break;
    }
    case 0x3:
        logger.error("[Interpreter] incorrect opcode %08x", instruction);
    }

    state.cpsr.c = carry;
    state.cpsr.z = state.gpr[rd] == 0;
    state.cpsr.n = state.gpr[rd] >> 31;

    state.gpr[15] += 2;
}

void Interpreter::thumb_alu_immediate() {
    u8 immediate = instruction & 0xFF;
    u8 rd = (instruction >> 8) & 0x7;
    u8 opcode = (instruction >> 11) & 0x3;

    switch (opcode) {
    case 0x0:
        state.gpr[rd] = immediate;
        state.cpsr.n = false;
        state.cpsr.z = state.gpr[rd] == 0;
        break;
    case 0x1:
        alu_cmp(state.gpr[rd], immediate);
        break;
    case 0x2:
        state.gpr[rd] = alu_add(state.gpr[rd], immediate, true);
        break;
    case 0x3:
        state.gpr[rd] = alu_sub(state.gpr[rd], immediate, true);
        break;
    default:
        logger.error("handle opcode %d", opcode);
    }

    state.gpr[15] += 2;
}

void Interpreter::thumb_data_processing_register() {
    u8 rd = instruction & 0x7;
    u8 rs = (instruction >> 3) & 0x7;
    u8 opcode = (instruction >> 6) & 0xF;
    bool carry = state.cpsr.c;

    switch (opcode) {
    case 0x0:
        state.gpr[rd] = alu_and(state.gpr[rd], state.gpr[rs], true);
        break;
    case 0x1:
        state.gpr[rd] = alu_eor(state.gpr[rd], state.gpr[rs], true);
        break;
    case 0x2:
        state.gpr[rd] = alu_lsl(state.gpr[rd], state.gpr[rs] & 0xFF, carry);
        state.cpsr.c = carry;
        state.cpsr.n = state.gpr[rd] >> 31;
        state.cpsr.z = state.gpr[rd] == 0;
        break;
    case 0x3:
        state.gpr[rd] = alu_lsr(state.gpr[rd], state.gpr[rs] & 0xFF, carry, false);
        state.cpsr.c = carry;
        state.cpsr.n = state.gpr[rd] >> 31;
        state.cpsr.z = state.gpr[rd] == 0;
        break;
    case 0x4:
        state.gpr[rd] = alu_asr(state.gpr[rd], state.gpr[rs] & 0xFF, carry, false);
        state.cpsr.c = carry;
        state.cpsr.n = state.gpr[rd] >> 31;
        state.cpsr.z = state.gpr[rd] == 0;
        break;
    case 0x5:
        state.gpr[rd] = alu_adc(state.gpr[rd], state.gpr[rs], true);
        break;
    case 0x6:
        state.gpr[rd] = alu_sbc(state.gpr[rd], state.gpr[rs], true);
        break;
    case 0x7:
        state.gpr[rd] = alu_ror(state.gpr[rd], state.gpr[rs] & 0xFF, carry, false);
        state.cpsr.c = carry;
        state.cpsr.n = state.gpr[rd] >> 31;
        state.cpsr.z = state.gpr[rd] == 0;
        break;
    case 0x8:
        alu_tst(state.gpr[rd], state.gpr[rs]);
        break;
    case 0x9:
        state.gpr[rd] = alu_sub(0, state.gpr[rs], true);
        break;
    case 0xA:
        alu_cmp(state.gpr[rd], state.gpr[rs]);
        break;
    case 0xB:
        alu_cmn(state.gpr[rd], state.gpr[rs]);
        break;
    case 0xC:
        state.gpr[rd] = alu_orr(state.gpr[rd], state.gpr[rs], true);
        break;
    case 0xD:
        state.gpr[rd] *= state.gpr[rs];

        state.cpsr.n = state.gpr[rd] >> 31;
        state.cpsr.z = state.gpr[rd] == 0;
        break;
    case 0xE:
        state.gpr[rd] = alu_bic(state.gpr[rd], state.gpr[rs], true);
        break;
    case 0xF:
        state.gpr[rd] = alu_mvn(state.gpr[rs], true);
        break;
    }

    state.gpr[15] += 2;
}

void Interpreter::thumb_special_data_processing() {
    u8 rd = ((instruction & (1 << 7)) >> 4) | (instruction & 0x7);
    u8 rs = (instruction >> 3) & 0xF;

    u8 opcode = (instruction >> 8) & 0x3;

    switch (opcode) {
    case 0x0:
        state.gpr[rd] += state.gpr[rs];
        if (rd == 15) {
            thumb_flush_pipeline();
        } else {
            state.gpr[15] += 2;    
        }

        break;
    case 0x1:
        alu_cmp(state.gpr[rd], state.gpr[rs]);
        state.gpr[15] += 2;
        break;
    case 0x2:
        state.gpr[rd] = state.gpr[rs];
        if (rd == 15) {
            thumb_flush_pipeline();
        } else {
            state.gpr[15] += 2;
        }

        break;
    default:
        logger.error("handle opcode %d", opcode);
    }
}

void Interpreter::thumb_adjust_stack_pointer() {
    u32 immediate = (instruction & 0x7F) << 2;

    // need to check bit 7 to check if we subtract or add from sp
    state.gpr[13] = state.gpr[13] + ((instruction & (1 << 7)) ? - immediate : immediate);

    state.gpr[15] += 2;
}

void Interpreter::thumb_add_sp_pc() {
    u32 immediate = (instruction & 0xFF) << 2;
    u8 rd = (instruction >> 8) & 0x7;
    bool sp = instruction & (1 << 11);

    if (sp) {
        state.gpr[rd] = state.gpr[13] + immediate;
    } else {
        state.gpr[rd] = (state.gpr[15] & ~0x2) + immediate;
    }

    state.gpr[15] += 2;
}

void Interpreter::thumb_branch_exchange() {
    // u8 rm = (instruction >> 3) & 0xF;
    // if (state.gpr[rm] & 0x1) {
    //     // just load rm into r15 normally in thumb
    //     state.gpr[15] = state.gpr[rm] & ~1;
    //     thumb_flush_pipeline();
    // } else {
    //     // switch to arm state
    //     // clear bit 5 in cpsr
    //     state.cpsr.t = false;
    //     state.gpr[15] = state.gpr[rm] & ~3;
    //     arm_flush_pipeline();
    // }
}

void Interpreter::thumb_branch_link_exchange() {
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
    // u32 immediate = ((instruction & (1 << 10)) ? 0xFFFFF000 : 0) | ((instruction & 0x7FF) << 1);

    // state.gpr[14] = state.gpr[15] + (immediate << 11);
    // state.gpr[15] += 2;
}

void Interpreter::thumb_branch_link_offset() {
    // u32 offset = (instruction & 0x7FF) << 1;
    // u32 next_instruction_address = state.gpr[15] - 1;

    // state.gpr[15] = (state.gpr[14] + offset) & ~1;
    // state.gpr[14] = next_instruction_address;
    // thumb_flush_pipeline();
}

void Interpreter::thumb_branch_link_exchange_offset() {
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
    // u32 offset = ((instruction & (1 << 10)) ? 0xFFFFF000 : 0) | ((instruction & 0x7FF) << 1);
    
    // state.gpr[15] += offset;
    // thumb_flush_pipeline();
}

void Interpreter::thumb_branch_conditional() {
    // u8 condition = (instruction >> 8) & 0xF;

    // if (evaluate_condition(condition)) {
    //     u32 offset = ((instruction & (1 << 7)) ? 0xFFFFFE00 : 0) | ((instruction & 0xFF) << 1);
    //     state.gpr[15] += offset;
    //     thumb_flush_pipeline();
    // } else {
    //     state.gpr[15] += 2;
    // }
}

void Interpreter::thumb_software_interrupt() {
    // state.spsr_banked[BANK_SVC].data = state.cpsr.data;

    // switch_mode(MODE_SVC);

    // state.cpsr.t = false;
    // state.cpsr.i = true;
    // state.gpr[14] = state.gpr[15] - 2;

    // // jump to the exception base in the bios
    // state.gpr[15] = coprocessor.get_exception_base() + 0x08;
    // arm_flush_pipeline();
}

void Interpreter::thumb_load_pc() {
    // u32 immediate = (m_instruction & 0xFF) << 2;
    // u8 rd = (m_instruction >> 8) & 0x7;

    // // bit 1 of pc is always set to 0
    // u32 address = (m_gpr[15] & ~0x2) + immediate;
    // m_gpr[rd] = read_word(address);
    
    // m_gpr[15] += 2;
}

void Interpreter::thumb_load_store() {
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
    // bool pclr = (m_instruction >> 8) & 0x1;
    // bool pop = (m_instruction >> 11) & 0x1;

    // u32 address = m_gpr[13];

    // if (pop) {
    //     for (int i = 0; i < 8; i++) {
    //         if (m_instruction & (1 << i)) {
    //             m_gpr[i] = read_word(address);
    //             address += 4;
    //         }
    //     }

    //     if (pclr) {
    //         m_gpr[15] = read_word(address);
    //         address += 4;

    //         if ((m_arch == Arch::ARMv4) || (m_gpr[15] & 0x1)) {
    //             // halfword align r15 and flush pipeline
    //             m_gpr[15] &= ~1;
    //             thumb_flush_pipeline();
    //         } else {
    //             // clear bit 5 of cpsr to switch to arm state
    //             m_cpsr.t = false;
    //             m_gpr[15] &= ~3;
    //             arm_flush_pipeline();
    //         }
    //     } else {
    //         m_gpr[15] += 2;
    //     }

    //     m_gpr[13] = address;
    // } else {
    //     for (int i = 0; i < 8; i++) {
    //         if (m_instruction & (1 << i)) {
    //             address -= 4;
    //         }
    //     }

    //     if (pclr) {
    //         address -= 4;
    //     }

    //     m_gpr[13] = address;

    //     for (int i = 0; i < 8; i++) {
    //         if (m_instruction & (1 << i)) {
    //             write_word(address, m_gpr[i]);
    //             address += 4;
    //         }
    //     }

    //     if (pclr) {
    //         write_word(address, m_gpr[14]);
    //     }

    //     m_gpr[15] += 2;
    // }
}

void Interpreter::thumb_load_store_sp_relative() {
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
    // u8 rd = m_instruction & 0x7;
    // u8 rn = (m_instruction >> 3) & 0x7;
    // u32 immediate = (m_instruction >> 6) & 0x1F;
    // u32 address = m_gpr[rn] + (immediate << 1);

    // bool load = m_instruction & (1 << 11);

    // if (load) {
    //     m_gpr[rd] = read_half(address);
    // } else {
    //     write_half(address, m_gpr[rd]);
    // }

    // m_gpr[15] += 2;
}

void Interpreter::thumb_load_store_multiple() {
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

} // namespace core::arm