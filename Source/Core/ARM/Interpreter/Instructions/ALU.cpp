#include "Common/Bits.h"
#include "Core/ARM/Interpreter/Interpreter.h"

void Interpreter::arm_count_leading_zeroes() {
    todo();
}

void Interpreter::arm_single_data_swap() {
    todo();
}

void Interpreter::arm_multiply() {
    todo();
}

void Interpreter::arm_saturating_add_subtract() {
    todo();
}

void Interpreter::arm_multiply_long() {
    todo();
}

void Interpreter::arm_data_processing() {
    const bool set_flags = (m_instruction >> 20) & 0x1;
    const bool immediate = (m_instruction >> 25) & 0x1;
    u8 rd = (m_instruction >> 12) & 0xF;
    u8 rn = (m_instruction >> 16) & 0xF;
    u8 opcode = (m_instruction >> 21) & 0xF;
    
    u32 op1 = m_gpr[rn];
    u32 op2 = 0;

    u8 carry_flag = m_cpsr.c;

    if (immediate) {
        u32 value = m_instruction & 0xFF;
        u8 shift_amount = ((m_instruction >> 8) & 0xF) << 1;

        op2 = Common::rotate_right(value, shift_amount);
        
        // carry flag is only affected if we have a non-zero shift amount
        if (shift_amount != 0) {
            carry_flag = op2 >> 31;
        }
    } else {
        u8 rm = m_instruction & 0xF;
        op2 = m_gpr[rm];
        u8 shift_type = (m_instruction >> 5) & 0x3;
        u8 shift_amount = 0;

        bool shift_imm = !(m_instruction & (1 << 4));

        if (shift_imm) {
            shift_amount = (m_instruction >> 7) & 0x1F;
        } else {
            u8 rs = (m_instruction >> 8) & 0xF;
            shift_amount = m_gpr[rs] & 0xFF;

            // if either rn or rm is r15 then
            // use as r15 + 12
            if (rn == 15) {
                op1 += 4;
            }

            if (rm == 15) {
                op2 += 4;
            }
        }

        op2 = arm_get_shifted_register_data_processing(op2, shift_type, shift_amount, carry_flag, shift_imm);
    }

    switch (opcode) {
    case 0x0:
        m_gpr[rd] = arm_and(op1, op2, set_flags);
        if (set_flags) {
            m_cpsr.c = carry_flag;
        }

        break;
    case 0x1:
        m_gpr[rd] = arm_eor(op1, op2, set_flags);
        if (set_flags) {
            m_cpsr.c = carry_flag;
        }

        break;
    case 0x2:
        m_gpr[rd] = arm_sub(op1, op2, set_flags);
        break;
    case 0x3:
        m_gpr[rd] = arm_sub(op2, op1, set_flags);
        break;
    case 0x4:
        m_gpr[rd] = arm_add(op1, op2, set_flags);
        break;
    case 0x5:
        m_gpr[rd] = arm_adc(op1, op2, set_flags);
        break;
    case 0x6:
        m_gpr[rd] = arm_sbc(op1, op2, set_flags);
        break;
    case 0x7:
        m_gpr[rd] = arm_sbc(op2, op1, set_flags);
        break;
    case 0x8:
        arm_tst(op1, op2);
        m_cpsr.c = carry_flag;
        break;
    case 0x9:
        arm_teq(op1, op2);
        m_cpsr.c = carry_flag;
        break;
    case 0xA:
        arm_cmp(op1, op2);
        break;
    case 0xB:
        arm_cmn(op1, op2);
        break;
    case 0xC:
        m_gpr[rd] = arm_orr(op1, op2, set_flags);
        if (set_flags) {
            m_cpsr.c = carry_flag;
        }

        break;
    case 0xD:
        m_gpr[rd] = arm_mov(op2, set_flags);
        if (set_flags) {
            m_cpsr.c = carry_flag;
        }
        
        break;
    case 0xE:
        m_gpr[rd] = arm_bic(op1, op2, set_flags);
        if (set_flags) {
            m_cpsr.c = carry_flag;
        }

        break;
    case 0xF:
        m_gpr[rd] = arm_mvn(op2, set_flags);
        if (set_flags) {
            m_cpsr.c = carry_flag;
        }

        break;
    }

    if (rd == 15) {
        if (set_flags) {
            // store the current spsr in cpsr only if in privileged mode
            if (is_privileged()) {
                u32 current_spsr = get_spsr();
                
                switch_mode(current_spsr & 0x1F);
                m_cpsr.data = current_spsr;
            } else {
                log_fatal("[ARM] Loading spsr into cpsr in non-privileged mode is undefined behaviour");
            }

            if (is_arm()) {
                arm_flush_pipeline();
            } else {
                thumb_flush_pipeline();
            }
        } else {
            arm_flush_pipeline();
        }
    } else {
        m_gpr[15] += 4;
    }
}

u32 Interpreter::arm_mov(u32 op2, u8 set_flags) {
    if (set_flags) {
        m_cpsr.n = op2 >> 31;
        m_cpsr.z = op2 == 0;
    }

    return op2;
}

u32 Interpreter::arm_mvn(u32 op2, u8 set_flags) {
    u32 result = ~op2;

    if (set_flags) {
        m_cpsr.n = result >> 31;
        m_cpsr.z = result == 0;
    }

    return result;
}

void Interpreter::arm_teq(u32 op1, u32 op2) {
    u32 result = op1 ^ op2;

    m_cpsr.n = result >> 31;
    m_cpsr.z = result == 0;
}

void Interpreter::arm_cmp(u32 op1, u32 op2) {
    u32 result = op1 - op2;

    m_cpsr.n = result >> 31;
    m_cpsr.z = result == 0;
    m_cpsr.c = op1 >= op2;
    m_cpsr.v = ((op1 ^ op2) & (op1 ^ result)) >> 31;
}

void Interpreter::arm_cmn(u32 op1, u32 op2) {
    u32 result = op1 + op2;

    m_cpsr.z = result == 0;
    m_cpsr.n = result >> 31;
    m_cpsr.c = result < op1;
    m_cpsr.v = (~(op1 ^ op2) & (op2 ^ result)) >> 31;
}

void Interpreter::arm_tst(u32 op1, u32 op2) {
    u32 result = op1 & op2;

    m_cpsr.z = result == 0;
    m_cpsr.n = result >> 31;
}

u32 Interpreter::arm_add(u32 op1, u32 op2, u8 set_flags) {
    u32 result = op1 + op2;

    if (set_flags) {
        m_cpsr.z = result == 0;
        m_cpsr.n = result >> 31;
        m_cpsr.c = result < op1;
        m_cpsr.v = (~(op1 ^ op2) & (op2 ^ result)) >> 31;
    }

    return result;
}

u32 Interpreter::arm_adc(u32 op1, u32 op2, u8 set_flags) {
    u64 result64 = (u64)op1 + (u64)op2 + (u64)m_cpsr.c;
    u32 result = (u32)result64;

    if (set_flags) {
        m_cpsr.z = result == 0;
        m_cpsr.n = result >> 31;
        m_cpsr.c = result64 >> 32;
        m_cpsr.v = (~(op1 ^ op2) & (op2 ^ result)) >> 31;
    }

    return result;
}

u32 Interpreter::arm_sbc(u32 op1, u32 op2, u8 set_flags) {
    u32 op3 = m_cpsr.c ^ 1;
    u32 result = op1 - op2 - op3;

    if (set_flags) {
        m_cpsr.n = result >> 31;
        m_cpsr.z = result == 0;
        m_cpsr.c = (u64)op1 >= (u64)op2 + (u64)op3;
        m_cpsr.v = ((op1 ^ op2) & (op1 ^ result)) >> 31;
    }

    return result;
}

u32 Interpreter::arm_eor(u32 op1, u32 op2, u8 set_flags) {
    u32 result = op1 ^ op2;
    
    if (set_flags) {
        m_cpsr.n = result >> 31;
        m_cpsr.z = result == 0;
    }

    return result;
}

u32 Interpreter::arm_sub(u32 op1, u32 op2, u8 set_flags) {
    u32 result = op1 - op2;
    
    if (set_flags) {
        m_cpsr.n = result >> 31;
        m_cpsr.z = result == 0;
        m_cpsr.c = op1 >= op2;
        m_cpsr.v = ((op1 ^ op2) & (op1 ^ result)) >> 31;
    }

    return result;
}

u32 Interpreter::arm_orr(u32 op1, u32 op2, u8 set_flags) {
    u32 result = op1 | op2;

    if (set_flags) {
        m_cpsr.n = result >> 31;
        m_cpsr.z = result == 0;
    }

    return result;
}

u32 Interpreter::arm_bic(u32 op1, u32 op2, u8 set_flags) {
    u32 result = op1 & ~op2;

    if (set_flags) {
        m_cpsr.n = result >> 31;
        m_cpsr.z = result == 0;
    }

    return result;
}

u32 Interpreter::arm_and(u32 op1, u32 op2, u8 set_flags) {
    u32 result = op1 & op2;

    if (set_flags) {
        m_cpsr.n = result >> 31;
        m_cpsr.z = result == 0;
    }

    return result;
}

u32 Interpreter::arm_get_shifted_register_data_processing(u32 op2, u8 shift_type, u8 shift_amount, u8& carry_flag, bool shift_imm) {
    switch (shift_type) {
    case 0x0:
        op2 = arm_lsl(op2, shift_amount, carry_flag);
        break;
    case 0x1:
        op2 = arm_lsr(op2, shift_amount, carry_flag, shift_imm);
        break;
    case 0x2:
        op2 = arm_asr(op2, shift_amount, carry_flag, shift_imm);
        break;
    case 0x3:
        op2 = arm_ror(op2, shift_amount, carry_flag, shift_imm);
        break;
    }

    return op2;
}

u32 Interpreter::arm_lsl(u32 op1, u8 shift_amount, u8& carry_flag) {
    if (shift_amount == 0) {
        // carry flag remains unchanged
        // and result is just rm
        return op1;
    }

    u32 result = 0;

    if (shift_amount >= 32) {
        result = 0;

        if (shift_amount > 32) {
            carry_flag = 0;
        } else {
            carry_flag = op1 & 0x1;
        }
    } else {
        result = op1 << shift_amount;
        carry_flag = (op1 >> (32 - shift_amount)) & 0x1;
    }

    return result;
}

u32 Interpreter::arm_lsr(u32 op1, u8 shift_amount, u8& carry_flag, bool shift_imm) {
    u32 result = 0;

    if (shift_imm) {
        if (shift_amount == 0) {
            result = 0;
            carry_flag = op1 >> 31;
        } else {
            result = op1 >> shift_amount;
            carry_flag = (op1 >> (shift_amount - 1)) & 0x1;
        }
    } else {
        if (shift_amount == 0) {
            result = op1;
        } else if (shift_amount < 32) {
            result = op1 >> shift_amount;
            carry_flag = (op1 >> (shift_amount - 1)) & 0x1;
        } else if (shift_amount == 32) {
            result = 0;
            carry_flag = op1 >> 31;
        } else {
            result = 0;
            carry_flag = 0;
        }
    }

    return result;
}

u32 Interpreter::arm_asr(u32 op1, u8 shift_amount, u8& carry_flag, bool shift_imm) {
    u32 result = 0;
    u8 msb = op1 >> 31;

    if (shift_imm) {
        if (shift_amount == 0) {
            result = 0xFFFFFFFF * msb;
            carry_flag = msb;
        } else {
            result = (op1 >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
            carry_flag = (op1 >> (shift_amount - 1)) & 0x1;
        }
    } else {
        if (shift_amount == 0) {
            result = op1;
        } else if (shift_amount < 32) {
            result = (op1 >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
            carry_flag = (op1 >> (shift_amount - 1)) & 0x1;
        } else {
            result = 0xFFFFFFFF * msb;
            carry_flag = msb;
        }
    }

    return result;
}

u32 Interpreter::arm_ror(u32 op1, u8 shift_amount, u8& carry_flag, bool shift_imm) {
    u32 result = 0;

    if (shift_imm) {
        if (shift_amount == 0) {
            result = (carry_flag << 31) | (op1 >> 1);
            carry_flag = op1 & 0x1;
        } else {
            result = Common::rotate_right(op1, shift_amount);
            carry_flag = (op1 >> (shift_amount - 1)) & 0x1;
        }
    } else {
        if (shift_amount == 0) {
            result = op1;
        } else if ((shift_amount & 0x1F) == 0) {
            result = op1;
            carry_flag = op1 >> 31;
        } else {
            result = Common::rotate_right(op1, shift_amount & 0x1F);
            carry_flag = (op1 >> ((shift_amount & 0x1F) - 1)) & 0x1;
        }
    }

    return result;
}

void Interpreter::arm_signed_halfword_accumulate_long() {
    todo();
}

void Interpreter::arm_signed_halfword_word_multiply() {
    todo();
}

void Interpreter::arm_signed_halfword_multiply() {
    todo();
}

void Interpreter::thumb_alu_immediate() {
    todo();
}

void Interpreter::thumb_data_processing_register() {
    todo();
}

void Interpreter::thumb_special_data_processing() {
    todo();
}

void Interpreter::thumb_add_subtract() {
    todo();
}

void Interpreter::thumb_shift_immediate() {
    todo();
}

void Interpreter::thumb_add_sp_pc() {
    todo();
}

void Interpreter::thumb_adjust_stack_pointer() {
    todo();
}
