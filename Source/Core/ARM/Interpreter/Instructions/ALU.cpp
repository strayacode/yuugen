#include "Common/Bits.h"
#include "Core/ARM/Interpreter/Interpreter.h"

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
        m_gpr[rd] = alu_and(op1, op2, set_flags);
        if (set_flags) {
            m_cpsr.c = carry_flag;
        }

        break;
    case 0x1:
        m_gpr[rd] = alu_eor(op1, op2, set_flags);
        if (set_flags) {
            m_cpsr.c = carry_flag;
        }

        break;
    case 0x2:
        m_gpr[rd] = alu_sub(op1, op2, set_flags);
        break;
    case 0x3:
        m_gpr[rd] = alu_sub(op2, op1, set_flags);
        break;
    case 0x4:
        m_gpr[rd] = alu_add(op1, op2, set_flags);
        break;
    case 0x5:
        m_gpr[rd] = alu_adc(op1, op2, set_flags);
        break;
    case 0x6:
        m_gpr[rd] = alu_sbc(op1, op2, set_flags);
        break;
    case 0x7:
        m_gpr[rd] = alu_sbc(op2, op1, set_flags);
        break;
    case 0x8:
        alu_tst(op1, op2);
        m_cpsr.c = carry_flag;
        break;
    case 0x9:
        alu_teq(op1, op2);
        m_cpsr.c = carry_flag;
        break;
    case 0xA:
        alu_cmp(op1, op2);
        break;
    case 0xB:
        alu_cmn(op1, op2);
        break;
    case 0xC:
        m_gpr[rd] = alu_orr(op1, op2, set_flags);
        if (set_flags) {
            m_cpsr.c = carry_flag;
        }

        break;
    case 0xD:
        m_gpr[rd] = alu_mov(op2, set_flags);
        if (set_flags) {
            m_cpsr.c = carry_flag;
        }
        
        break;
    case 0xE:
        m_gpr[rd] = alu_bic(op1, op2, set_flags);
        if (set_flags) {
            m_cpsr.c = carry_flag;
        }

        break;
    case 0xF:
        m_gpr[rd] = alu_mvn(op2, set_flags);
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

u32 Interpreter::alu_mov(u32 op2, u8 set_flags) {
    if (set_flags) {
        m_cpsr.n = op2 >> 31;
        m_cpsr.z = op2 == 0;
    }

    return op2;
}

u32 Interpreter::alu_mvn(u32 op2, u8 set_flags) {
    u32 result = ~op2;

    if (set_flags) {
        m_cpsr.n = result >> 31;
        m_cpsr.z = result == 0;
    }

    return result;
}

void Interpreter::alu_teq(u32 op1, u32 op2) {
    u32 result = op1 ^ op2;

    m_cpsr.n = result >> 31;
    m_cpsr.z = result == 0;
}

void Interpreter::alu_cmp(u32 op1, u32 op2) {
    u32 result = op1 - op2;

    m_cpsr.n = result >> 31;
    m_cpsr.z = result == 0;
    m_cpsr.c = op1 >= op2;
    m_cpsr.v = ((op1 ^ op2) & (op1 ^ result)) >> 31;
}

void Interpreter::alu_cmn(u32 op1, u32 op2) {
    u32 result = op1 + op2;

    m_cpsr.z = result == 0;
    m_cpsr.n = result >> 31;
    m_cpsr.c = result < op1;
    m_cpsr.v = (~(op1 ^ op2) & (op2 ^ result)) >> 31;
}

void Interpreter::alu_tst(u32 op1, u32 op2) {
    u32 result = op1 & op2;

    m_cpsr.z = result == 0;
    m_cpsr.n = result >> 31;
}

u32 Interpreter::alu_add(u32 op1, u32 op2, u8 set_flags) {
    u32 result = op1 + op2;

    if (set_flags) {
        m_cpsr.z = result == 0;
        m_cpsr.n = result >> 31;
        m_cpsr.c = result < op1;
        m_cpsr.v = (~(op1 ^ op2) & (op2 ^ result)) >> 31;
    }

    return result;
}

u32 Interpreter::alu_adc(u32 op1, u32 op2, u8 set_flags) {
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

u32 Interpreter::alu_sbc(u32 op1, u32 op2, u8 set_flags) {
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

u32 Interpreter::alu_eor(u32 op1, u32 op2, u8 set_flags) {
    u32 result = op1 ^ op2;
    
    if (set_flags) {
        m_cpsr.n = result >> 31;
        m_cpsr.z = result == 0;
    }

    return result;
}

u32 Interpreter::alu_sub(u32 op1, u32 op2, u8 set_flags) {
    u32 result = op1 - op2;
    
    if (set_flags) {
        m_cpsr.n = result >> 31;
        m_cpsr.z = result == 0;
        m_cpsr.c = op1 >= op2;
        m_cpsr.v = ((op1 ^ op2) & (op1 ^ result)) >> 31;
    }

    return result;
}

u32 Interpreter::alu_orr(u32 op1, u32 op2, u8 set_flags) {
    u32 result = op1 | op2;

    if (set_flags) {
        m_cpsr.n = result >> 31;
        m_cpsr.z = result == 0;
    }

    return result;
}

u32 Interpreter::alu_bic(u32 op1, u32 op2, u8 set_flags) {
    u32 result = op1 & ~op2;

    if (set_flags) {
        m_cpsr.n = result >> 31;
        m_cpsr.z = result == 0;
    }

    return result;
}

u32 Interpreter::alu_and(u32 op1, u32 op2, u8 set_flags) {
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
        op2 = alu_lsl(op2, shift_amount, carry_flag);
        break;
    case 0x1:
        op2 = alu_lsr(op2, shift_amount, carry_flag, shift_imm);
        break;
    case 0x2:
        op2 = alu_asr(op2, shift_amount, carry_flag, shift_imm);
        break;
    case 0x3:
        op2 = alu_ror(op2, shift_amount, carry_flag, shift_imm);
        break;
    }

    return op2;
}

u32 Interpreter::alu_lsl(u32 op1, u8 shift_amount, u8& carry_flag) {
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

u32 Interpreter::alu_lsr(u32 op1, u8 shift_amount, u8& carry_flag, bool shift_imm) {
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

u32 Interpreter::alu_asr(u32 op1, u8 shift_amount, u8& carry_flag, bool shift_imm) {
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

u32 Interpreter::alu_ror(u32 op1, u8 shift_amount, u8& carry_flag, bool shift_imm) {
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

void Interpreter::arm_multiply() {
    const bool set_flags = (m_instruction >> 20) & 0x1;
    const bool accumulate = (m_instruction >> 21) & 0x1;
    u8 rm = m_instruction & 0xF;
    u8 rs = (m_instruction >> 8) & 0xF;
    u8 rn = (m_instruction >> 12) & 0xF;
    u8 rd = (m_instruction >> 16) & 0xF;
    u32 result = m_gpr[rm] * m_gpr[rs];

    if (accumulate) {
        result += m_gpr[rn];
    }

    m_gpr[rd] = result;

    if (set_flags) {
        m_cpsr.n = m_gpr[rd] >> 31;
        m_cpsr.z = m_gpr[rd] == 0;
    }

    m_gpr[15] += 4;
}

void Interpreter::arm_multiply_long() {
    const bool set_flags = (m_instruction >> 20) & 0x1;
    const bool accumulate = (m_instruction >> 21) & 0x1;
    const bool sign = (m_instruction >> 22) & 0x1;
    u8 rm = m_instruction & 0xF;
    u8 rs = (m_instruction >> 8) & 0xF;
    u8 rdlo = (m_instruction >> 12) & 0xF;
    u8 rdhi = (m_instruction >> 16) & 0xF;

    s64 result = 0;

    if (sign) {
        result = (s64)(s32)(m_gpr[rm]) * (s64)(s32)(m_gpr[rs]);
    } else {
        u64 temp_result = (u64)m_gpr[rm] * (u64)m_gpr[rs];
        result = (s64)temp_result;
    }

    if (accumulate) {
        s64 temp_result = ((u64)m_gpr[rdhi] << 32) | ((u64)m_gpr[rdlo]);
        result += temp_result;
    }

    if (set_flags) {
        m_cpsr.n = result >> 63;
        m_cpsr.z = result == 0;
    }

    m_gpr[rdhi] = result >> 32;
    m_gpr[rdlo] = result & 0xFFFFFFFF;

    m_gpr[15] += 4;
}

void Interpreter::arm_single_data_swap() {
    u8 rm = m_instruction & 0xF;
    u8 rd = (m_instruction >> 12) & 0xF;
    u8 rn = (m_instruction >> 16) & 0xF;
    u8 byte = (m_instruction >> 22) & 0x1;
    u32 address = m_gpr[rn];
    u32 data = 0;

    if (byte) {
        data = read_byte(address);
    } else {
        data = read_word(address);
    }

    if (address & 0x3) {
        int shift_amount = (address & 0x3) * 8;
        data = Common::rotate_right(data, shift_amount);
    }

    if (byte) {
        write_byte(address, m_gpr[rm]);
    } else {
        write_word(address, m_gpr[rm]);
    }

    m_gpr[rd] = data;
    m_gpr[15] += 4;
}

void Interpreter::arm_count_leading_zeroes() {
    if (m_arch == Arch::ARMv4) {
        return;
    }

    u8 rm = m_instruction & 0xF;
    u8 rd = (m_instruction >> 12) & 0xF;
    u32 data = m_gpr[rm];
    u32 count = 0;

    while (data != 0) {
        data >>= 1;
        count++;
    }

    m_gpr[rd] = 32 - count;
    m_gpr[15] += 4;
}

void Interpreter::arm_saturating_add_subtract() {
    if (m_arch == Arch::ARMv4) {
        return;
    }

    u8 rm = m_instruction & 0xF;
    u8 rd = (m_instruction >> 12) & 0xF;
    u8 rn = (m_instruction >> 16) & 0xF;
    u8 opcode = (m_instruction >> 20) & 0xF;

    u32 result = 0;

    switch (opcode) {
    case 0x0:
        result = m_gpr[rm] + m_gpr[rn];
        if ((~(m_gpr[rm] ^ m_gpr[rn]) & (m_gpr[rn] ^ result)) >> 31) {
            // set q flag
            m_cpsr.q = true;

            // saturate the result
            // this approach avoids an if else statement
            result = 0x80000000 - (result >> 31);
        }
        break;
    case 0x2:
        result = m_gpr[rm] - m_gpr[rn];
        if (((m_gpr[rm] ^ m_gpr[rn]) & (m_gpr[rm] ^ result)) >> 31) {
            // set q flag
            m_cpsr.q = true;

            // since a signed overflow occured with saturated arithmetic, we set the result to the max value
            // according to if its the max negative value (-2^31, 0x80000000) or positive value (2^31 - 1, 0x7FFFFFFF)
            // if greater than the largest positive value (2^31 - 1)
            // saturate the result
            // this approach avoids an if else statement
            result = 0x80000000 - (result >> 31);
        }

        break;
    case 0x4: {
        result = m_gpr[rn] * 2;

        if ((m_gpr[rn] ^ result) >> 31) {
            // if the last bit has changed then we know a signed overflow occured
            m_cpsr.q = true;

            // saturate the result
            // this approach avoids an if else statement
            result = 0x80000000 - (result >> 31);
        }

        u32 old_result = result;
        result += m_gpr[rm];
        if ((~(old_result ^ m_gpr[rm]) & (m_gpr[rm] ^ result)) >> 31) {
            // set q flag
            m_cpsr.q = true;

            // since a signed overflow occured with saturated arithmetic, we set the result to the max value
            // according to if its the max negative value (-2^31, 0x80000000) or positive value (2^31 - 1, 0x7FFFFFFF)
            // if greater than the largest positive value (2^31 - 1)
            // saturate the result
            // this approach avoids an if else statement
            result = 0x80000000 - (result >> 31);
        }

        break;
    }
    case 0x6: {
        result = m_gpr[rn] * 2;

        if ((m_gpr[rn] ^ result) >> 31) {
            // set q flag
            m_cpsr.q = true;

            // since a signed overflow occured with saturated arithmetic, we set the result to the max value
            // according to if its the max negative value (-2^31, 0x80000000) or positive value (2^31 - 1, 0x7FFFFFFF)
            // if greater than the largest positive value (2^31 - 1)
            // saturate the result
            // this approach avoids an if else statement
            result = 0x80000000 - (result >> 31);
        }

        u32 old_result = result;
        // now subtract rm
        result = m_gpr[rm] - result;
        if (((m_gpr[rm] ^ old_result) & (m_gpr[rm] ^ result)) >> 31) {
            // set q flag
            m_cpsr.q = true;

            // since a signed overflow occured with saturated arithmetic, we set the result to the max value
            // according to if its the max negative value (-2^31, 0x80000000) or positive value (2^31 - 1, 0x7FFFFFFF)
            // if greater than the largest positive value (2^31 - 1)
            // saturate the result
            // this approach avoids an if else statement
            result = 0x80000000 - (result >> 31);
        }

        break;
    }
    default:
        log_fatal("handle opcode %d", opcode);
    }
    
    if (rd == 15) {
        todo();
    }

    m_gpr[rd] = result;
    m_gpr[15] += 4;
}

void Interpreter::arm_signed_halfword_multiply() {
    if (m_arch == Arch::ARMv4) {
        return;
    }

    if (((m_instruction >> 21) & 0xF) == 0xA) {
        todo();
    }

    bool accumulate = ((m_instruction >> 21) & 0x3) == 0;
    u8 op1 = m_instruction & 0xF;
    u8 op2 = (m_instruction >> 8) & 0xF;
    u8 op3 = (m_instruction >> 12) & 0xF;
    u8 op4 = (m_instruction >> 16) & 0xF;

    bool x = m_instruction & (1 << 5);
    bool y = m_instruction & (1 << 6);

    s16 result1;
    s16 result2;

    if (x) {
        result1 = (s16)(m_gpr[op1] >> 16);
    } else {
        result1 = (s16)m_gpr[op1];
    }

    if (y) {
        result2 = (s16)(m_gpr[op2] >> 16);
    } else {
        result2 = (s16)m_gpr[op2];
    }

    u32 result = result1 * result2;

    if (accumulate) {
        u32 operand = m_gpr[op3];

        m_gpr[op4] = result + operand;

        if ((~(result ^ operand) & (operand ^ m_gpr[op4])) >> 31) {
            m_cpsr.q = true;
        }
    } else {
        m_gpr[op4] = result;
    }

    m_gpr[15] += 4;
}

void Interpreter::arm_signed_halfword_word_multiply() {
    if (m_arch == Arch::ARMv4) {
        return;
    }

    u8 op1 = m_instruction & 0xF;
    u8 op2 = (m_instruction >> 8) & 0xF;
    u8 op3 = (m_instruction >> 12) & 0xF;
    u8 op4 = (m_instruction >> 16) & 0xF;

    bool x = m_instruction & (1 << 5);
    bool y = m_instruction & (1 << 6);

    u32 result;

    if (y) {
        result = ((s32)m_gpr[op1] * (s16)(m_gpr[op2] >> 16)) >> 16;
    } else {
        result = ((s32)m_gpr[op1] * (s16)m_gpr[op2]) >> 16;
    }

    if (!x) {
        u32 operand = m_gpr[op3];

        m_gpr[op4] = result + operand;

        if ((~(result ^ operand) & (operand ^ m_gpr[op4])) >> 31) {
            m_cpsr.q = true;
        }
    } else {
        m_gpr[op4] = result;
    }

    m_gpr[15] += 4;
}

void Interpreter::arm_signed_halfword_accumulate_long() {
    if (m_arch == Arch::ARMv4) {
        return;
    }

    u8 op1 = m_instruction & 0xF;
    u8 op2 = (m_instruction >> 8) & 0xF;
    u8 op3 = (m_instruction >> 12) & 0xF;
    u8 op4 = (m_instruction >> 16) & 0xF;

    bool x = m_instruction & (1 << 5);
    bool y = m_instruction & (1 << 6);

    s64 rdhilo = (s64)(((u64)m_gpr[op4] << 32) | ((u64)m_gpr[op3]));

    s64 result1;
    s64 result2;

    if (x) {
        result1 = (s64)(s16)(m_gpr[op1] >> 16);
    } else {
        result1 = (s64)(s16)m_gpr[op1];
    }

    if (y) {
        result2 = (s64)(s16)(m_gpr[op2] >> 16);
    } else {
        result2 = (s64)(s16)m_gpr[op2];
    }

    s64 result = result1 * result2;
    result += rdhilo;
    m_gpr[op3] = result;
    m_gpr[op4] = result >> 32;

    m_gpr[15] += 4;
}

void Interpreter::thumb_add_subtract() {
    u8 rn = (m_instruction >> 6) & 0x7;
    u8 rs = (m_instruction >> 3) & 0x7;
    u8 rd = m_instruction & 0x7;

    bool immediate = m_instruction & (1 << 10);
    bool sub = m_instruction & (1 << 9);

    u32 op1 = m_gpr[rs];
    u32 op2 = immediate ? rn : m_gpr[rn];

    if (sub) {
        m_gpr[rd] = alu_sub(op1, op2, true);
    } else {
        m_gpr[rd] = alu_add(op1, op2, true);
    }

    m_gpr[15] += 2;
}

void Interpreter::thumb_shift_immediate() {
    u8 rd = m_instruction & 0x7;
    u8 rs = (m_instruction >> 3) & 0x7;

    u8 shift_amount = (m_instruction >> 6) & 0x1F;
    u8 shift_type = (m_instruction >> 11) & 0x3;

    u8 carry = m_cpsr.c;

    switch (shift_type) {
    case 0x0:
        if (shift_amount != 0) {
            carry = (m_gpr[rs] >> (32 - shift_amount)) & 0x1;
        }

        m_gpr[rd] = m_gpr[rs] << shift_amount;
        break;
    case 0x1:
        if (shift_amount == 0) {
            carry = m_gpr[rs] >> 31;
            m_gpr[rd] = 0;
        } else {
            carry = (m_gpr[rs] >> (shift_amount - 1)) & 0x1;
            m_gpr[rd] = m_gpr[rs] >> shift_amount;
        } 
        break;
    case 0x2: {
        u32 msb = m_gpr[rs] >> 31;

        if (shift_amount == 0) {
            carry = m_gpr[rd] >> 31;
            m_gpr[rd] = 0xFFFFFFFF * msb;
        } else {
            carry = (m_gpr[rs] >> (shift_amount - 1)) & 0x1;
            m_gpr[rd] = (m_gpr[rs] >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
        }
        break;
    }
    case 0x3:
        log_fatal("[Interpreter] incorrect opcode %08x", m_instruction);
    }

    m_cpsr.c = carry;
    m_cpsr.z = m_gpr[rd] == 0;
    m_cpsr.n = m_gpr[rd] >> 31;

    m_gpr[15] += 2;
}

void Interpreter::thumb_alu_immediate() {
    u8 immediate = m_instruction & 0xFF;
    u8 rd = (m_instruction >> 8) & 0x7;
    u8 opcode = (m_instruction >> 11) & 0x3;

    switch (opcode) {
    case 0x0:
        m_gpr[rd] = immediate;
        m_cpsr.n = false;
        m_cpsr.z = m_gpr[rd] == 0;
        break;
    case 0x1:
        alu_cmp(m_gpr[rd], immediate);
        break;
    case 0x2:
        m_gpr[rd] = alu_add(m_gpr[rd], immediate, true);
        break;
    case 0x3:
        m_gpr[rd] = alu_sub(m_gpr[rd], immediate, true);
        break;
    default:
        log_fatal("handle opcode %d", opcode);
    }

    m_gpr[15] += 2;
}

void Interpreter::thumb_data_processing_register() {
    u8 rd = m_instruction & 0x7;
    u8 rs = (m_instruction >> 3) & 0x7;
    u8 opcode = (m_instruction >> 6) & 0xF;
    u8 carry = m_cpsr.c;

    switch (opcode) {
    case 0x0:
        m_gpr[rd] = alu_and(m_gpr[rd], m_gpr[rs], true);
        break;
    case 0x1:
        m_gpr[rd] = alu_eor(m_gpr[rd], m_gpr[rs], true);
        break;
    case 0x2:
        m_gpr[rd] = alu_lsl(m_gpr[rd], m_gpr[rs] & 0xFF, carry);
        m_cpsr.c = carry;
        m_cpsr.n = m_gpr[rd] >> 31;
        m_cpsr.z = m_gpr[rd] == 0;
        break;
    case 0x3:
        m_gpr[rd] = alu_lsr(m_gpr[rd], m_gpr[rs] & 0xFF, carry, false);
        m_cpsr.c = carry;
        m_cpsr.n = m_gpr[rd] >> 31;
        m_cpsr.z = m_gpr[rd] == 0;
        break;
    case 0x4:
        m_gpr[rd] = alu_asr(m_gpr[rd], m_gpr[rs] & 0xFF, carry, false);
        m_cpsr.c = carry;
        m_cpsr.n = m_gpr[rd] >> 31;
        m_cpsr.z = m_gpr[rd] == 0;
        break;
    case 0x5:
        m_gpr[rd] = alu_adc(m_gpr[rd], m_gpr[rs], true);
        break;
    case 0x6:
        m_gpr[rd] = alu_sbc(m_gpr[rd], m_gpr[rs], true);
        break;
    case 0x7:
        m_gpr[rd] = alu_ror(m_gpr[rd], m_gpr[rs] & 0xFF, carry, false);
        m_cpsr.c = carry;
        m_cpsr.n = m_gpr[rd] >> 31;
        m_cpsr.z = m_gpr[rd] == 0;
        break;
    case 0x8:
        alu_tst(m_gpr[rd], m_gpr[rs]);
        break;
    case 0x9:
        m_gpr[rd] = alu_sub(0, m_gpr[rs], true);
        break;
    case 0xA:
        alu_cmp(m_gpr[rd], m_gpr[rs]);
        break;
    case 0xB:
        alu_cmn(m_gpr[rd], m_gpr[rs]);
        break;
    case 0xC:
        m_gpr[rd] = alu_orr(m_gpr[rd], m_gpr[rs], true);
        break;
    case 0xD:
        m_gpr[rd] *= m_gpr[rs];

        m_cpsr.n = m_gpr[rd] >> 31;
        m_cpsr.z = m_gpr[rd] == 0;
        break;
    case 0xE:
        m_gpr[rd] = alu_bic(m_gpr[rd], m_gpr[rs], true);
        break;
    case 0xF:
        m_gpr[rd] = alu_mvn(m_gpr[rs], true);
        break;
    }

    m_gpr[15] += 2;
}

void Interpreter::thumb_special_data_processing() {
    u8 rd = ((m_instruction & (1 << 7)) >> 4) | (m_instruction & 0x7);
    u8 rs = (m_instruction >> 3) & 0xF;

    u8 opcode = (m_instruction >> 8) & 0x3;

    switch (opcode) {
    case 0x0:
        m_gpr[rd] += m_gpr[rs];
        if (rd == 15) {
            thumb_flush_pipeline();
        } else {
            m_gpr[15] += 2;    
        }

        break;
    case 0x1:
        alu_cmp(m_gpr[rd], m_gpr[rs]);
        m_gpr[15] += 2;
        break;
    case 0x2:
        m_gpr[rd] = m_gpr[rs];
        if (rd == 15) {
            thumb_flush_pipeline();
        } else {
            m_gpr[15] += 2;
        }

        break;
    default:
        log_fatal("handle opcode %d", opcode);
    }
}

void Interpreter::thumb_adjust_stack_pointer() {
    u32 immediate = (m_instruction & 0x7F) << 2;

    // need to check bit 7 to check if we subtract or add from sp
    m_gpr[13] = m_gpr[13] + ((m_instruction & (1 << 7)) ? - immediate : immediate);

    m_gpr[15] += 2;
}

void Interpreter::thumb_add_sp_pc() {
    u32 immediate = (m_instruction & 0xFF) << 2;
    u8 rd = (m_instruction >> 8) & 0x7;
    bool sp = m_instruction & (1 << 11);

    if (sp) {
        m_gpr[rd] = m_gpr[13] + immediate;
    } else {
        m_gpr[rd] = (m_gpr[15] & ~0x2) + immediate;
    }

    m_gpr[15] += 2;
}
