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

IRResultAndCarry IREmitter::create_result_and_carry() {
    return IRResultAndCarry{create_variable(), create_variable()};
}

IRVariable IREmitter::load_gpr(GPR gpr) {
    auto dst = create_variable();
    GuestRegister src{gpr, basic_block.location.get_mode()};
    push<IRLoadGPR>(dst, src);
    return dst;
}

void IREmitter::store_gpr(GPR gpr, IRValue src) {
    GuestRegister dst{gpr, basic_block.location.get_mode()};
    push<IRStoreGPR>(dst, src);
}

IRVariable IREmitter::load_cpsr() {
    auto dst = create_variable();
    push<IRLoadCPSR>(dst);
    return dst;
}

void IREmitter::store_cpsr(IRVariable src) {
    push<IRStoreCPSR>(src);
}

IRVariable IREmitter::load_spsr() {
    auto dst = create_variable();
    push<IRLoadSPSR>(dst);
    return dst;
}

void IREmitter::store_spsr(IRVariable src) {
    push<IRStoreSPSR>(src);
}

void IREmitter::copy_spsr_to_cpsr() {
    auto spsr = load_spsr();
    store_cpsr(spsr);
}

IRVariable IREmitter::bitwise_and(IRValue lhs, IRValue rhs) {
    auto dst = create_variable();
    push<IRBitwiseAnd>(dst, lhs, rhs);
    return dst;
}

IRVariable IREmitter::bitwise_or(IRValue lhs, IRValue rhs) {
    auto dst = create_variable();
    push<IRBitwiseOr>(dst, lhs, rhs);
    return dst;
}

IRVariable IREmitter::bitwise_not(IRValue src) {
    auto dst = create_variable();
    push<IRBitwiseNot>(dst, src);
    return dst;
}

IRVariable IREmitter::bitwise_exclusive_or(IRValue lhs, IRValue rhs) {
    auto dst = create_variable();
    push<IRBitwiseExclusiveOr>(dst, lhs, rhs);
    return dst;
}

IRVariable IREmitter::add(IRValue lhs, IRValue rhs) {
    auto dst = create_variable();
    push<IRAdd>(dst, lhs, rhs);
    return dst;
}

IRVariable IREmitter::subtract(IRValue lhs, IRValue rhs) {
    auto dst = create_variable();
    push<IRSubtract>(dst, lhs, rhs);
    return dst;
}

IRVariable IREmitter::multiply(IRValue lhs, IRValue rhs) {
    auto dst = create_variable();
    push<IRMultiply>(dst, lhs, rhs);
    return dst;
}

IRVariable IREmitter::logical_shift_left(IRValue src, IRValue amount) {
    auto dst = create_variable();
    push<IRLogicalShiftLeft>(dst, src, amount);
    return dst;
}

IRVariable IREmitter::logical_shift_right(IRValue src, IRValue amount) {
    auto dst = create_variable();
    push<IRLogicalShiftRight>(dst, src, amount);
    return dst;
}

IRResultAndCarry IREmitter::barrel_shifter_logical_shift_left(IRValue src, IRValue amount) {
    auto result_and_carry = create_result_and_carry();
    push<IRBarrelShifterLogicalShiftLeft>(result_and_carry, src, amount, load_flag(Flag::C));
    return result_and_carry;
}

IRResultAndCarry IREmitter::barrel_shifter_logical_shift_right(IRValue src, IRValue amount) {
    auto result_and_carry = create_result_and_carry();
    push<IRBarrelShifterLogicalShiftRight>(result_and_carry, src, amount, load_flag(Flag::C));
    return result_and_carry;
}

IRResultAndCarry IREmitter::barrel_shifter_arithmetic_shift_right(IRValue src, IRValue amount) {
    auto result_and_carry = create_result_and_carry();
    push<IRBarrelShifterArithmeticShiftRight>(result_and_carry, src, amount, load_flag(Flag::C));
    return result_and_carry;
}

IRResultAndCarry IREmitter::barrel_shifter_rotate_right(IRValue src, IRValue amount) {
    auto result_and_carry = create_result_and_carry();
    push<IRBarrelShifterRotateRight>(result_and_carry, src, amount, load_flag(Flag::C));
    return result_and_carry;
}

IRResultAndCarry IREmitter::barrel_shifter_rotate_right_extended(IRValue src, IRConstant amount) {
    auto result_and_carry = create_result_and_carry();
    push<IRBarrelShifterRotateRightExtended>(result_and_carry, src, amount, load_flag(Flag::C));
    return result_and_carry;
}

IRVariable IREmitter::load_flag(Flag flag) {
    auto cpsr = load_cpsr();
    return bitwise_and(logical_shift_right(cpsr, constant(flag)), constant(1));
}

void IREmitter::store_flag(Flag flag, IRValue value) {
    auto cpsr = load_cpsr();
    auto cpsr_masked = bitwise_and(cpsr, bitwise_not(constant(1 << flag)));
    auto value_shifted = logical_shift_left(value, constant(flag));
    store_cpsr(bitwise_or(cpsr_masked, value_shifted));
}

void IREmitter::store_nz(IRValue value) {
    store_flag(Flag::N, logical_shift_right(value, constant(31)));
    store_flag(Flag::Z, compare(value, constant(0), CompareType::Equal));
}

void IREmitter::store_add_cv(IRValue lhs, IRValue rhs, IRValue result) {
    store_flag(Flag::C, compare(result, lhs, CompareType::LessThan));
    store_flag(Flag::V, add_overflow(lhs, rhs, result));
}

void IREmitter::store_sub_cv(IRValue lhs, IRValue rhs, IRValue result) {
    store_flag(Flag::C, compare(lhs, rhs, CompareType::GreaterEqual));
    store_flag(Flag::V, sub_overflow(lhs, rhs, result));
}

IRVariable IREmitter::add_overflow(IRValue lhs, IRValue rhs, IRValue result) {
    auto a = bitwise_not(bitwise_exclusive_or(lhs, rhs));
    auto b = bitwise_exclusive_or(rhs, result);
    return logical_shift_right(bitwise_and(a, b), constant(31));
}

IRVariable IREmitter::sub_overflow(IRValue lhs, IRValue rhs, IRValue result) {
    auto a = bitwise_exclusive_or(lhs, rhs);
    auto b = bitwise_exclusive_or(lhs, result);
    return logical_shift_right(bitwise_and(a, b), constant(31));
}

IRVariable IREmitter::compare(IRValue lhs, IRValue rhs, CompareType compare_type) {
    auto dst = create_variable();
    push<IRCompare>(dst, lhs, rhs, compare_type);
    return dst;
}

void IREmitter::branch(IRValue address) {
    auto instruction_size = basic_block.location.get_instruction_size();
    auto address_mask = ~(instruction_size - 1);
    auto adjusted_address = add(bitwise_and(address, constant(address_mask)), constant(2 * instruction_size));
    store_gpr(GPR::PC, adjusted_address);
}

void IREmitter::branch_exchange(IRValue address, ExchangeType exchange_type) {
    if (exchange_type != ExchangeType::Bit0) {
        logger.todo("handle different exchange type");
    }

    auto bit0 = bitwise_and(address, constant(1));
    store_flag(Flag::T, bit0);

    auto address_mask = bitwise_not(subtract(constant(3), multiply(constant(2), bit0)));
    auto adjusted_address = bitwise_and(address, address_mask);
    store_gpr(GPR::PC, adjusted_address);
}

IRVariable IREmitter::copy(IRValue src) {
    auto dst = create_variable();
    push<IRCopy>(dst, src);
    return dst;
}

IRConstant IREmitter::constant(u32 value) {
    return IRConstant{value};
}

IRResultAndCarry IREmitter::barrel_shifter(IRValue value, ShiftType shift_type, IRValue amount) {
    switch (shift_type) {
    case ShiftType::LSL:
        return barrel_shifter_logical_shift_left(value, amount);
    case ShiftType::LSR:
        return barrel_shifter_logical_shift_right(value, amount);
    case ShiftType::ASR:
        return barrel_shifter_arithmetic_shift_right(value, amount);
    case ShiftType::ROR:
        if (amount.is_constant() && amount.as_constant().value == 0) {
            return barrel_shifter_rotate_right_extended(value, constant(1));
        } else {
            return barrel_shifter_rotate_right(value, amount);
        }
    }
}

void IREmitter::link() {
    auto address = basic_block.current_address;
    auto instruction_size = basic_block.location.get_instruction_size();
    store_gpr(GPR::LR, constant(address + instruction_size));
}

void IREmitter::advance_pc() {
    auto address = basic_block.current_address;
    auto instruction_size = basic_block.location.get_instruction_size();
    store_gpr(GPR::PC, constant(address + instruction_size));
}

void IREmitter::memory_write(IRValue addr, IRVariable src, AccessSize access_size, AccessType access_type) {
    push<IRMemoryWrite>(addr, src, access_size, access_type);
}

IRVariable IREmitter::memory_read(IRValue addr, AccessSize access_size, AccessType access_type) {
    auto dst = create_variable();
    push<IRMemoryRead>(dst, addr, access_size, access_type);
    return dst;
}

IRVariable IREmitter::add_carry(IRValue lhs, IRValue rhs, bool set_flags) {
    auto dst = create_variable();
    push<IRAddCarry>(dst, lhs, rhs, set_flags);
    return dst;
}

} // namespace arm