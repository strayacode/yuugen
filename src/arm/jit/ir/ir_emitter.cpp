#include <cassert>
#include "common/logger.h"
#include "arm/jit/ir/opcodes.h"
#include "arm/jit/ir/ir_emitter.h"

namespace arm {

IREmitter::IREmitter(BasicBlock& basic_block) : basic_block(basic_block) {}

IRVariable IREmitter::create_variable() {
    IRVariable variable{next_variable_id};
    next_variable_id++;
    return variable;
}

IRPair<IRVariable> IREmitter::create_pair() {
    return IRPair{create_variable(), create_variable()};
}

TypedValue<Type::U32> IREmitter::load_gpr(GPR gpr) {
    auto dst = create_variable();
    GuestRegister src{gpr, basic_block.location.get_mode()};
    push<IRLoadGPR>(dst, src);
    return TypedValue<Type::U32>{dst};
}

TypedValue<Type::U32> IREmitter::load_gpr(GPR gpr, Mode mode) {
    auto dst = create_variable();
    GuestRegister src{gpr, mode};
    push<IRLoadGPR>(dst, src);
    return TypedValue<Type::U32>{dst};
}

void IREmitter::store_gpr(GPR gpr, TypedValue<Type::U32> src) {
    GuestRegister dst{gpr, basic_block.location.get_mode()};
    push<IRStoreGPR>(dst, src);
}

void IREmitter::store_gpr(GPR gpr, Mode mode, TypedValue<Type::U32> src) {
    GuestRegister dst{gpr, mode};
    push<IRStoreGPR>(dst, src);
}

TypedValue<Type::U32> IREmitter::load_cpsr() {
    auto dst = create_variable();
    push<IRLoadCPSR>(dst);
    return TypedValue<Type::U32>{dst};
}

void IREmitter::store_cpsr(TypedValue<Type::U32> src) {
    push<IRStoreCPSR>(src);
}

TypedValue<Type::U32> IREmitter::load_spsr() {
    auto dst = create_variable();
    push<IRLoadSPSR>(dst, basic_block.location.get_mode());
    return TypedValue<Type::U32>{dst};
}

void IREmitter::store_spsr(TypedValue<Type::U32> src) {
    push<IRStoreSPSR>(src, basic_block.location.get_mode());
}

void IREmitter::store_spsr(TypedValue<Type::U32> src, Mode mode) {
    push<IRStoreSPSR>(src, mode);
}

TypedValue<Type::U32> IREmitter::load_coprocessor(u32 cn, u32 cm, u32 cp) {
    auto dst = create_variable();
    push<IRLoadCoprocessor>(dst, cn, cm, cp);
    return TypedValue<Type::U32>{dst};
}

void IREmitter::store_coprocessor(u32 cn, u32 cm, u32 cp, TypedValue<Type::U32> src) {
    push<IRStoreCoprocessor>(cn, cm, cp, src);
}

void IREmitter::copy_spsr_to_cpsr() {
    auto spsr = load_spsr();
    store_cpsr(spsr);
}

TypedValue<Type::U32> IREmitter::bitwise_and(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs) {
    auto dst = create_variable();
    push<IRBitwiseAnd>(dst, lhs, rhs);
    return TypedValue<Type::U32>{dst};
}

TypedValue<Type::U32> IREmitter::bitwise_or(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs) {
    auto dst = create_variable();
    push<IRBitwiseOr>(dst, lhs, rhs);
    return TypedValue<Type::U32>{dst};
}

TypedValue<Type::U32> IREmitter::bitwise_not(TypedValue<Type::U32> src) {
    auto dst = create_variable();
    push<IRBitwiseNot>(dst, src);
    return TypedValue<Type::U32>{dst};
}

TypedValue<Type::U32> IREmitter::bitwise_exclusive_or(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs) {
    auto dst = create_variable();
    push<IRBitwiseExclusiveOr>(dst, lhs.value, rhs.value);
    return TypedValue<Type::U32>(dst);
}

TypedValue<Type::U32> IREmitter::add(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs) {
    auto dst = create_variable();
    push<IRAdd>(dst, lhs, rhs);
    return TypedValue<Type::U32>{dst};
}

IRPair<IRVariable> IREmitter::add_long(IRPair<IRValue> lhs, IRPair<IRValue> rhs) {
    auto dst = create_pair();
    push<IRAddLong>(dst, lhs, rhs);
    return dst;
}

TypedValue<Type::U32> IREmitter::subtract(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs) {
    auto dst = create_variable();
    push<IRSubtract>(dst, lhs, rhs);
    return TypedValue<Type::U32>{dst};
}

TypedValue<Type::U32> IREmitter::subtract_with_carry(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs, TypedValue<Type::U1> carry) {
    const auto flipped = bitwise_exclusive_or(extend32(carry), imm32(1));
    return subtract(subtract(lhs, rhs));
}

TypedValue<Type::U32> IREmitter::multiply(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs) {
    auto dst = create_variable();
    push<IRMultiply>(dst, lhs, rhs);
    return TypedValue<Type::U32>{dst};
}

IRPair<IRVariable> IREmitter::multiply_long(IRValue lhs, IRValue rhs, bool is_signed) {
    auto dst = create_pair();
    push<IRMultiplyLong>(dst, lhs, rhs, is_signed);
    return dst;
}

TypedValue<Type::U32> IREmitter::logical_shift_left(TypedValue<Type::U32> src, TypedValue<Type::U8> amount) {
    auto dst = create_variable();
    push<IRLogicalShiftLeft>(dst, src, amount);
    return TypedValue<Type::U32>{dst};
}

TypedValue<Type::U32> IREmitter::logical_shift_right(TypedValue<Type::U32> src, TypedValue<Type::U8> amount) {
    auto dst = create_variable();
    push<IRLogicalShiftRight>(dst, src, amount);
    return TypedValue<Type::U32>{dst};
}

TypedValue<Type::U32> IREmitter::arithmetic_shift_right(TypedValue<Type::U32> src, TypedValue<Type::U8> amount) {
    auto dst = create_variable();
    push<IRArithmeticShiftRight>(dst, src, amount);
    return TypedValue<Type::U32>{dst};
}

TypedValue<Type::U32> IREmitter::rotate_right(TypedValue<Type::U32> src, TypedValue<Type::U8> amount) {
    auto dst = create_variable();
    push<IRRotateRight>(dst, src, amount);
    return TypedValue<Type::U32>{dst};
}

IRPair<IRVariable> IREmitter::barrel_shifter_logical_shift_left(IRValue src, IRValue amount) {
    auto result_and_carry = create_pair();
    push<IRBarrelShifterLogicalShiftLeft>(result_and_carry, src, amount, load_flag(Flag::C));
    return result_and_carry;
}

IRPair<IRVariable> IREmitter::barrel_shifter_logical_shift_right(IRValue src, IRValue amount) {
    auto result_and_carry = create_pair();
    push<IRBarrelShifterLogicalShiftRight>(result_and_carry, src, amount, load_flag(Flag::C), amount.is_constant());
    return result_and_carry;
}

IRPair<IRVariable> IREmitter::barrel_shifter_arithmetic_shift_right(IRValue src, IRValue amount) {
    auto result_and_carry = create_pair();
    push<IRBarrelShifterArithmeticShiftRight>(result_and_carry, src, amount, load_flag(Flag::C), amount.is_constant());
    return result_and_carry;
}

IRPair<IRVariable> IREmitter::barrel_shifter_rotate_right(IRValue src, TypedValue<Type::U8> amount) {
    auto result_and_carry = create_pair();
    push<IRBarrelShifterRotateRight>(result_and_carry, src, amount, load_flag(Flag::C));
    return result_and_carry;
}

IRPair<IRVariable> IREmitter::barrel_shifter_rotate_right_extended(IRValue src, TypedValue<Type::U8> amount) {
    auto result_and_carry = create_pair();
    push<IRBarrelShifterRotateRightExtended>(result_and_carry, src, amount, load_flag(Flag::C));
    return result_and_carry;
}

IRVariable IREmitter::sign_extend_byte(IRValue src) {
    auto shifted_left = logical_shift_left(src, constant(24));
    return arithmetic_shift_right(shifted_left, constant(24));
}

IRVariable IREmitter::sign_extend_half(IRValue src) {
    auto shifted_left = logical_shift_left(src, constant(16));
    return arithmetic_shift_right(shifted_left, constant(16));
}

IRVariable IREmitter::count_leading_zeroes(IRValue src) {
    auto dst = create_variable();
    push<IRCountLeadingZeroes>(dst, src);
    return dst;
}

TypedValue<Type::U1> IREmitter::load_flag(Flag flag) {
    auto cpsr = load_cpsr();
    return get_bit(cpsr, constant(flag));
}

void IREmitter::store_flag(Flag flag, TypedValue<Type::U1> value) {
    auto cpsr = load_cpsr();
    auto cpsr_with_flag = set_bit(cpsr, value, imm8(flag));
    store_cpsr(cpsr_with_flag);
}

void IREmitter::store_nz(TypedValue<Type::U32> value) {
    store_flag(Flag::N, get_bit(value, imm8(31)));
    store_flag(Flag::Z, compare(value, imm32(0), CompareType::Equal));
}

void IREmitter::store_nz_long(IRPair<TypedValue<Type::U32>> value) {
    store_flag(Flag::N, get_bit(value.first, imm8(31)));
    store_flag(Flag::Z, truncate1(bitwise_and(extend32(compare(value.first, imm32(0), CompareType::Equal)), extend32(compare(value.second, imm32(0), CompareType::Equal)))));
}

void IREmitter::store_add_cv(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs, TypedValue<Type::U32> result) {
    store_flag(Flag::C, compare(result, lhs, CompareType::LessThan));
    store_flag(Flag::V, add_overflow(lhs, rhs, result));
}

void IREmitter::store_sub_cv(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs, TypedValue<Type::U32> result) {
    store_flag(Flag::C, compare(lhs, rhs, CompareType::GreaterEqual));
    store_flag(Flag::V, sub_overflow(lhs, rhs, result));
}

void IREmitter::store_adc_cv(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs, TypedValue<Type::U32> result) {
    auto carry = load_flag(Flag::C);
    auto a = bitwise_and(extend32(compare(lhs, result, CompareType::Equal)), extend32(carry));
    auto b = extend32(compare(lhs, result, CompareType::GreaterThan));

    store_flag(Flag::C, truncate1(bitwise_or(a, b)));
    store_flag(Flag::V, add_overflow(lhs, rhs, result));
}

void IREmitter::store_sbc_cv(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs, TypedValue<Type::U32> result) {
    auto carry = load_flag(Flag::C);
    auto a = bitwise_and(extend32(compare(lhs, rhs, CompareType::Equal)), extend32(carry));
    auto b = extend32(compare(lhs, rhs, CompareType::GreaterThan));

    store_flag(Flag::C, truncate1(bitwise_or(a, b)));
    store_flag(Flag::V, sub_overflow(lhs, rhs, result));
}

TypedValue<Type::U1> IREmitter::add_overflow(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs, TypedValue<Type::U32> result) {
    auto a = bitwise_not(bitwise_exclusive_or(lhs, rhs));
    auto b = bitwise_exclusive_or(rhs, result);
    return get_bit(bitwise_and(a, b), imm8(31));
}

TypedValue<Type::U1> IREmitter::sub_overflow(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs, TypedValue<Type::U32> result) {
    auto a = bitwise_exclusive_or(lhs, rhs);
    auto b = bitwise_exclusive_or(lhs, result);
    return get_bit(bitwise_and(a, b), imm8(31));
}

TypedValue<Type::U1> IREmitter::compare(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs, CompareType compare_type) {
    auto dst = create_variable();
    push<IRCompare>(dst, lhs, rhs, compare_type);
    return TypedValue<Type::U1>{dst};
}

void IREmitter::branch(TypedValue<Type::U32> address) {
    auto instruction_size = basic_block.location.get_instruction_size();
    auto address_mask = ~(instruction_size - 1);
    auto adjusted_address = bitwise_and(address, imm32(address_mask));
    store_gpr(GPR::PC, adjusted_address);
}

void IREmitter::branch_exchange(TypedValue<Type::U32> address, ExchangeType exchange_type) {
    TypedValue<Type::U1> thumb;

    switch (exchange_type) {
    case ExchangeType::Bit0:
        thumb = get_bit(address, imm8(1));
        store_flag(Flag::T, thumb);
        break;
    case ExchangeType::ThumbBit:
        thumb = load_flag(Flag::T);
        break;
    }

    auto address_mask = bitwise_not(subtract(imm32(3), multiply(imm32(2), extend32(thumb))));
    auto address_offset = subtract(imm32(8), multiply(imm32(4), extend32(thumb)));
    auto adjusted_address = bitwise_and(add(address, address_offset), address_mask);
    store_gpr(GPR::PC, adjusted_address);
}

TypedValue<Type::U32> IREmitter::copy(TypedValue<Type::U32> src) {
    auto dst = create_variable();
    push<IRCopy>(dst, src);
    return TypedValue<Type::U32>{dst};
}

TypedValue<Type::U1> IREmitter::get_bit(TypedValue<Type::U32> src, TypedValue<Type::U8> bit) {
    auto dst = create_variable();
    push<IRGetBit>(dst, src, bit);
    return TypedValue<Type::U1>{dst};
}

TypedValue<Type::U1> IREmitter::set_bit(TypedValue<Type::U32> src, TypedValue<Type::U1> value, TypedValue<Type::U8> bit) {
    auto dst = create_variable();
    push<IRSetBit>(dst, src, value, bit);
    return TypedValue<Type::U1>{dst};
}

TypedValue<Type::U1> IREmitter::imm1(bool value) {
    return TypedValue<Type::U1>(value);
}

TypedValue<Type::U8> IREmitter::imm8(u8 value) {
    return TypedValue<Type::U8>(value);
}

TypedValue<Type::U32> IREmitter::imm32(u32 value) {
    return TypedValue<Type::U32>(value);
}

IRPair<IRValue> IREmitter::pair(IRValue first, IRValue second) {
    return IRPair{first, second};
}

IRPair<IRVariable> IREmitter::barrel_shifter(IRValue value, ShiftType shift_type, IRValue amount) {
    switch (shift_type) {
    case ShiftType::LSL:
        return barrel_shifter_logical_shift_left(value, amount);
    case ShiftType::LSR:
        return barrel_shifter_logical_shift_right(value, amount);
    case ShiftType::ASR:
        return barrel_shifter_arithmetic_shift_right(value, amount);
    case ShiftType::ROR:
        if (amount.is_constant() && amount.as_constant().value == 0) {
            return barrel_shifter_rotate_right_extended(value, imm8(1));
        } else {
            return barrel_shifter_rotate_right(value, amount);
        }
    }
}

void IREmitter::link() {
    auto address = basic_block.current_address;
    auto instruction_size = basic_block.location.get_instruction_size();

    if (basic_block.location.is_arm()) {
        store_gpr(GPR::LR, imm32(address + instruction_size));
    } else {
        store_gpr(GPR::LR, imm32((address + instruction_size) | 0x1));
    }
}

void IREmitter::advance_pc() {
    auto address = basic_block.current_address;
    auto instruction_size = basic_block.location.get_instruction_size();
    store_gpr(GPR::PC, imm32(address + (3 * instruction_size)));
}

void IREmitter::flush_pipeline() {
    auto instruction_size = basic_block.location.get_instruction_size();
    auto address_mask = ~(instruction_size - 1);
    auto pc = load_gpr(GPR::PC);
    auto adjusted_pc = bitwise_and(pc, imm32(address_mask));
    store_gpr(GPR::PC, add(adjusted_pc, imm32(2 * instruction_size)));
}

void IREmitter::memory_write_byte(TypedValue<Type::U32> addr, TypedValue<Type::U8> src) {
    push<IRMemoryWrite>(addr, src, AccessSize::Byte);
}

void IREmitter::memory_write_half(TypedValue<Type::U32> addr, TypedValue<Type::U16> src) {
    push<IRMemoryWrite>(addr, src, AccessSize::Half);
}

void IREmitter::memory_write_word(TypedValue<Type::U32> addr, TypedValue<Type::U32> src) {
    push<IRMemoryWrite>(addr, src, AccessSize::Word);
}

IRVariable IREmitter::memory_read(IRValue addr, AccessSize access_size, AccessType access_type) {
    auto dst = create_variable();
    push<IRMemoryRead>(dst, addr, access_size, access_type);
    return dst;
}

} // namespace arm