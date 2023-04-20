#include "common/bits.h"
#include "core/arm/instructions.h"
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
            carry = common::get_bit(op2, 31);
        }
    } else {
        int amount = 0;
        op2 = state.gpr[rhs.reg.rm];

        if (opcode.rhs.reg.imm) {
            amount = rhs.reg.amount.imm;
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
            //     log_fatal("[ARM] Loading spsr into cpsr in non-privileged mode is undefined behaviour");
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

u32 Interpreter::barrel_shifter(u32 value, ShiftType shift_type, int amount, bool& carry, bool imm) {
    switch (shift_type) {
    case ShiftType::LSL:
        return alu_lsl(value, amount, carry);
    case ShiftType::LSR:
        return alu_lsr(value, amount, carry, imm);
    case ShiftType::ASR:
        return alu_asr(value, amount, carry, imm);
    case ShiftType::ROR:
        return alu_ror(value, amount, carry, imm);
    }
    
    return value;
}

u32 Interpreter::alu_mov(u32 op2, u8 set_flags) {
    if (set_flags) {
        state.cpsr.n = op2 >> 31;
        state.cpsr.z = op2 == 0;
    }

    return op2;
}

u32 Interpreter::alu_mvn(u32 op2, u8 set_flags) {
    u32 result = ~op2;

    if (set_flags) {
        state.cpsr.n = result >> 31;
        state.cpsr.z = result == 0;
    }

    return result;
}

void Interpreter::alu_teq(u32 op1, u32 op2) {
    u32 result = op1 ^ op2;

    state.cpsr.n = result >> 31;
    state.cpsr.z = result == 0;
}

void Interpreter::alu_cmp(u32 op1, u32 op2) {
    u32 result = op1 - op2;

    state.cpsr.n = result >> 31;
    state.cpsr.z = result == 0;
    state.cpsr.c = op1 >= op2;
    state.cpsr.v = ((op1 ^ op2) & (op1 ^ result)) >> 31;
}

void Interpreter::alu_cmn(u32 op1, u32 op2) {
    u32 result = op1 + op2;

    state.cpsr.z = result == 0;
    state.cpsr.n = result >> 31;
    state.cpsr.c = result < op1;
    state.cpsr.v = (~(op1 ^ op2) & (op2 ^ result)) >> 31;
}

void Interpreter::alu_tst(u32 op1, u32 op2) {
    u32 result = op1 & op2;

    state.cpsr.z = result == 0;
    state.cpsr.n = result >> 31;
}

u32 Interpreter::alu_add(u32 op1, u32 op2, u8 set_flags) {
    u32 result = op1 + op2;

    if (set_flags) {
        state.cpsr.z = result == 0;
        state.cpsr.n = result >> 31;
        state.cpsr.c = result < op1;
        state.cpsr.v = (~(op1 ^ op2) & (op2 ^ result)) >> 31;
    }

    return result;
}

u32 Interpreter::alu_adc(u32 op1, u32 op2, u8 set_flags) {
    u64 result64 = (u64)op1 + (u64)op2 + (u64)state.cpsr.c;
    u32 result = (u32)result64;

    if (set_flags) {
        state.cpsr.z = result == 0;
        state.cpsr.n = result >> 31;
        state.cpsr.c = result64 >> 32;
        state.cpsr.v = (~(op1 ^ op2) & (op2 ^ result)) >> 31;
    }

    return result;
}

u32 Interpreter::alu_sbc(u32 op1, u32 op2, u8 set_flags) {
    u32 op3 = state.cpsr.c ^ 1;
    u32 result = op1 - op2 - op3;

    if (set_flags) {
        state.cpsr.n = result >> 31;
        state.cpsr.z = result == 0;
        state.cpsr.c = (u64)op1 >= (u64)op2 + (u64)op3;
        state.cpsr.v = ((op1 ^ op2) & (op1 ^ result)) >> 31;
    }

    return result;
}

u32 Interpreter::alu_eor(u32 op1, u32 op2, u8 set_flags) {
    u32 result = op1 ^ op2;
    
    if (set_flags) {
        state.cpsr.n = result >> 31;
        state.cpsr.z = result == 0;
    }

    return result;
}

u32 Interpreter::alu_sub(u32 op1, u32 op2, u8 set_flags) {
    u32 result = op1 - op2;
    
    if (set_flags) {
        state.cpsr.n = result >> 31;
        state.cpsr.z = result == 0;
        state.cpsr.c = op1 >= op2;
        state.cpsr.v = ((op1 ^ op2) & (op1 ^ result)) >> 31;
    }

    return result;
}

u32 Interpreter::alu_orr(u32 op1, u32 op2, u8 set_flags) {
    u32 result = op1 | op2;

    if (set_flags) {
        state.cpsr.n = result >> 31;
        state.cpsr.z = result == 0;
    }

    return result;
}

u32 Interpreter::alu_bic(u32 op1, u32 op2, u8 set_flags) {
    u32 result = op1 & ~op2;

    if (set_flags) {
        state.cpsr.n = result >> 31;
        state.cpsr.z = result == 0;
    }

    return result;
}

u32 Interpreter::alu_and(u32 op1, u32 op2, u8 set_flags) {
    u32 result = op1 & op2;

    if (set_flags) {
        state.cpsr.n = result >> 31;
        state.cpsr.z = result == 0;
    }

    return result;
}

u32 Interpreter::alu_lsl(u32 value, int amount, bool& carry) {
    if (amount == 0) {
        return value;
    }

    u32 result = 0;
    if (amount >= 32) {
        if (amount > 32) {
            carry = 0;
        } else {
            carry = value & 0x1;
        }
    } else {
        result = value << amount;
        carry = (value >> (32 - amount)) & 0x1;
    }

    return result;
}

u32 Interpreter::alu_lsr(u32 value, int amount, bool& carry, bool imm) {
    u32 result = 0;

    if (imm) {
        if (amount == 0) {
            result = 0;
            carry = op1 >> 31;
        } else {
            result = op1 >> amount;
            carry = (op1 >> (amount - 1)) & 0x1;
        }
    } else {
        if (amount == 0) {
            result = op1;
        } else if (amount < 32) {
            result = op1 >> amount;
            carry = (op1 >> (amount - 1)) & 0x1;
        } else if (amount == 32) {
            result = 0;
            carry = op1 >> 31;
        } else {
            result = 0;
            carry = false;
        }
    }

    return result;
}

u32 Interpreter::alu_asr(u32 value, int amount, bool& carry, bool imm) {
    u32 result = 0;
    u8 msb = op1 >> 31;

    if (imm) {
        if (amount == 0) {
            result = 0xffffffff * msb;
            carry = msb;
        } else {
            result = (op1 >> amount) | ((0xffffffff * msb) << (32 - amount));
            carry = (op1 >> (amount - 1)) & 0x1;
        }
    } else {
        if (amount == 0) {
            result = op1;
        } else if (amount < 32) {
            result = (op1 >> amount) | ((0xffffffff * msb) << (32 - amount));
            carry = (op1 >> (amount - 1)) & 0x1;
        } else {
            result = 0xffffffff * msb;
            carry = msb;
        }
    }

    return result;
}

u32 Interpreter::alu_ror(u32 value, int amount, bool& carry, bool imm) {
    u32 result = 0;

    if (imm) {
        if (amount == 0) {
            result = (carry << 31) | (op1 >> 1);
            carry = op1 & 0x1;
        } else {
            result = common::rotate_right(op1, amount);
            carry = (op1 >> (amount - 1)) & 0x1;
        }
    } else {
        if (amount == 0) {
            result = op1;
        } else if ((amount & 0x1f) == 0) {
            result = op1;
            carry = op1 >> 31;
        } else {
            result = common::rotate_right(op1, amount & 0x1f);
            carry = (op1 >> ((amount & 0x1f) - 1)) & 0x1;
        }
    }

    return result;
}

void Interpreter::arm_multiply() {
    const bool set_flags = (instruction >> 20) & 0x1;
    const bool accumulate = (instruction >> 21) & 0x1;
    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rn = (instruction >> 12) & 0xF;
    u8 rd = (instruction >> 16) & 0xF;
    u32 result = state.gpr[rm] * state.gpr[rs];

    if (accumulate) {
        result += state.gpr[rn];
    }

    state.gpr[rd] = result;

    if (set_flags) {
        state.cpsr.n = state.gpr[rd] >> 31;
        state.cpsr.z = state.gpr[rd] == 0;
    }

    state.gpr[15] += 4;
}

void Interpreter::arm_multiply_long() {
    const bool set_flags = (instruction >> 20) & 0x1;
    const bool accumulate = (instruction >> 21) & 0x1;
    const bool sign = (instruction >> 22) & 0x1;
    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rdlo = (instruction >> 12) & 0xF;
    u8 rdhi = (instruction >> 16) & 0xF;

    s64 result = 0;

    if (sign) {
        result = (s64)(s32)(state.gpr[rm]) * (s64)(s32)(state.gpr[rs]);
    } else {
        u64 temp_result = (u64)state.gpr[rm] * (u64)state.gpr[rs];
        result = (s64)temp_result;
    }

    if (accumulate) {
        s64 temp_result = ((u64)state.gpr[rdhi] << 32) | ((u64)state.gpr[rdlo]);
        result += temp_result;
    }

    if (set_flags) {
        state.cpsr.n = result >> 63;
        state.cpsr.z = result == 0;
    }

    state.gpr[rdhi] = result >> 32;
    state.gpr[rdlo] = result & 0xFFFFFFFF;

    state.gpr[15] += 4;
}

void Interpreter::arm_single_data_swap() {
    u8 rm = instruction & 0xF;
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    u8 byte = (instruction >> 22) & 0x1;
    u32 address = state.gpr[rn];
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
        write_byte(address, state.gpr[rm]);
    } else {
        write_word(address, state.gpr[rm]);
    }

    state.gpr[rd] = data;
    state.gpr[15] += 4;
}

void Interpreter::arm_count_leading_zeroes() {
    if (arch == Arch::ARMv4) {
        return;
    }

    u8 rm = instruction & 0xF;
    u8 rd = (instruction >> 12) & 0xF;
    u32 data = state.gpr[rm];
    u32 count = 0;

    while (data != 0) {
        data >>= 1;
        count++;
    }

    state.gpr[rd] = 32 - count;
    state.gpr[15] += 4;
}

void Interpreter::arm_saturating_add_subtract() {
    if (arch == Arch::ARMv4) {
        return;
    }

    u8 rm = instruction & 0xF;
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    u8 opcode = (instruction >> 20) & 0xF;

    u32 result = 0;

    switch (opcode) {
    case 0x0:
        result = state.gpr[rm] + state.gpr[rn];
        if ((~(state.gpr[rm] ^ state.gpr[rn]) & (state.gpr[rn] ^ result)) >> 31) {
            // set q flag
            state.cpsr.q = true;

            // saturate the result
            // this approach avoids an if else statement
            result = 0x80000000 - (result >> 31);
        }
        break;
    case 0x2:
        result = state.gpr[rm] - state.gpr[rn];
        if (((state.gpr[rm] ^ state.gpr[rn]) & (state.gpr[rm] ^ result)) >> 31) {
            // set q flag
            state.cpsr.q = true;

            // since a signed overflow occured with saturated arithmetic, we set the result to the max value
            // according to if its the max negative value (-2^31, 0x80000000) or positive value (2^31 - 1, 0x7FFFFFFF)
            // if greater than the largest positive value (2^31 - 1)
            // saturate the result
            // this approach avoids an if else statement
            result = 0x80000000 - (result >> 31);
        }

        break;
    case 0x4: {
        result = state.gpr[rn] * 2;

        if ((state.gpr[rn] ^ result) >> 31) {
            // if the last bit has changed then we know a signed overflow occured
            state.cpsr.q = true;

            // saturate the result
            // this approach avoids an if else statement
            result = 0x80000000 - (result >> 31);
        }

        u32 old_result = result;
        result += state.gpr[rm];
        if ((~(old_result ^ state.gpr[rm]) & (state.gpr[rm] ^ result)) >> 31) {
            // set q flag
            state.cpsr.q = true;

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
        result = state.gpr[rn] * 2;

        if ((state.gpr[rn] ^ result) >> 31) {
            // set q flag
            state.cpsr.q = true;

            // since a signed overflow occured with saturated arithmetic, we set the result to the max value
            // according to if its the max negative value (-2^31, 0x80000000) or positive value (2^31 - 1, 0x7FFFFFFF)
            // if greater than the largest positive value (2^31 - 1)
            // saturate the result
            // this approach avoids an if else statement
            result = 0x80000000 - (result >> 31);
        }

        u32 old_result = result;
        // now subtract rm
        result = state.gpr[rm] - result;
        if (((state.gpr[rm] ^ old_result) & (state.gpr[rm] ^ result)) >> 31) {
            // set q flag
            state.cpsr.q = true;

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

    state.gpr[rd] = result;
    state.gpr[15] += 4;
}

void Interpreter::arm_signed_halfword_multiply() {
    if (arch == Arch::ARMv4) {
        return;
    }

    if (((instruction >> 21) & 0xF) == 0xA) {
        todo();
    }

    bool accumulate = ((instruction >> 21) & 0x3) == 0;
    u8 op1 = instruction & 0xF;
    u8 op2 = (instruction >> 8) & 0xF;
    u8 op3 = (instruction >> 12) & 0xF;
    u8 op4 = (instruction >> 16) & 0xF;

    bool x = instruction & (1 << 5);
    bool y = instruction & (1 << 6);

    s16 result1;
    s16 result2;

    if (x) {
        result1 = (s16)(state.gpr[op1] >> 16);
    } else {
        result1 = (s16)state.gpr[op1];
    }

    if (y) {
        result2 = (s16)(state.gpr[op2] >> 16);
    } else {
        result2 = (s16)state.gpr[op2];
    }

    u32 result = result1 * result2;

    if (accumulate) {
        u32 operand = state.gpr[op3];

        state.gpr[op4] = result + operand;

        if ((~(result ^ operand) & (operand ^ state.gpr[op4])) >> 31) {
            state.cpsr.q = true;
        }
    } else {
        state.gpr[op4] = result;
    }

    state.gpr[15] += 4;
}

void Interpreter::arm_signed_halfword_word_multiply() {
    if (arch == Arch::ARMv4) {
        return;
    }

    u8 op1 = instruction & 0xF;
    u8 op2 = (instruction >> 8) & 0xF;
    u8 op3 = (instruction >> 12) & 0xF;
    u8 op4 = (instruction >> 16) & 0xF;

    bool x = instruction & (1 << 5);
    bool y = instruction & (1 << 6);

    u32 result;

    if (y) {
        result = ((s32)state.gpr[op1] * (s16)(state.gpr[op2] >> 16)) >> 16;
    } else {
        result = ((s32)state.gpr[op1] * (s16)state.gpr[op2]) >> 16;
    }

    if (!x) {
        u32 operand = state.gpr[op3];

        state.gpr[op4] = result + operand;

        if ((~(result ^ operand) & (operand ^ state.gpr[op4])) >> 31) {
            state.cpsr.q = true;
        }
    } else {
        state.gpr[op4] = result;
    }

    state.gpr[15] += 4;
}

void Interpreter::arm_signed_halfword_accumulate_long() {
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
        log_fatal("[Interpreter] incorrect opcode %08x", instruction);
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
        log_fatal("handle opcode %d", opcode);
    }

    state.gpr[15] += 2;
}

void Interpreter::thumb_data_processing_register() {
    u8 rd = instruction & 0x7;
    u8 rs = (instruction >> 3) & 0x7;
    u8 opcode = (instruction >> 6) & 0xF;
    u8 carry = state.cpsr.c;

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
        log_fatal("handle opcode %d", opcode);
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

} // namespace core::arm