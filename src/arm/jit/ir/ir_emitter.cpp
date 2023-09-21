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

IRVariable IREmitter::sub(IRValue lhs, IRValue rhs) {
    auto dst = create_variable();
    push<IRSub>(dst, lhs, rhs);
    return dst;
}

void IREmitter::store_flag(Flag flag, IRValue value) {
    auto cpsr = load_cpsr();
    auto cpsr_with_cleared_flag = bitwise_and(cpsr, bitwise_not(constant(flag)));
    auto cpsr_with_set_flag = bitwise_or(cpsr_with_cleared_flag, constant(flag));
    store_cpsr(cpsr_with_set_flag);
}

void IREmitter::store_nz(IRValue value) {
    store_flag(Flag::N, compare(value, constant(0), CompareType::IntegerLessThan));
    store_flag(Flag::Z, compare(value, constant(0), CompareType::Equal));
}

void IREmitter::store_add_cv(IRValue lhs, IRValue rhs, IRValue result) {
    store_flag(Flag::C, compare(result, lhs, CompareType::LessThan));
    store_flag(Flag::V, add_overflow(lhs, rhs, result));
}

IRVariable IREmitter::add_overflow(IRValue lhs, IRValue rhs, IRValue result) {
    auto a = bitwise_not(bitwise_exclusive_or(lhs, rhs));
    auto b = bitwise_exclusive_or(rhs, result);
    return compare(a, b, CompareType::IntegerLessThan);
}

IRVariable IREmitter::compare(IRValue lhs, IRValue rhs, CompareType compare_type) {
    auto dst = create_variable();
    push<IRCompare>(dst, lhs, rhs, compare_type);
    return dst;
}

void IREmitter::branch(IRValue address, bool is_arm) {
    push<IRBranch>(address, is_arm);
}

void IREmitter::branch_exchange(IRValue address, ExchangeType exchange_type) {
    push<IRBranchExchange>(address, exchange_type);
}

IRVariable IREmitter::copy(IRValue src) {
    auto dst = create_variable();
    push<IRCopy>(dst, src);
    return dst;
}

IRConstant IREmitter::constant(u32 value) {
    return IRConstant{value};
}

IRVariable IREmitter::barrel_shifter(IRValue value, ShiftType shift_type, IRValue amount) {
    switch (shift_type) {
    case ShiftType::LSL:
        return logical_shift_left(value, amount);
    case ShiftType::LSR:
        return logical_shift_right(value, amount);
    case ShiftType::ASR:
        return arithmetic_shift_right(value, amount);
    case ShiftType::ROR:
        return rotate_right(value, amount);
    }
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

void IREmitter::memory_write(IRValue addr, IRVariable src, AccessSize access_size, AccessType access_type) {
    push<IRMemoryWrite>(addr, src, access_size, access_type);
}

IRVariable IREmitter::arithmetic_shift_right(IRValue src, IRValue amount) {
    auto dst = create_variable();
    push<IRArithmeticShiftRight>(dst, src, amount);
    return dst;
}

IRVariable IREmitter::rotate_right(IRValue src, IRValue amount) {
    auto dst = create_variable();
    push<IRRotateRight>(dst, src, amount);
    return dst;
}

IRVariable IREmitter::memory_read(IRValue addr, AccessSize access_size, AccessType access_type) {
    auto dst = create_variable();
    push<IRMemoryRead>(dst, addr, access_size, access_type);
    return dst;
}

IRVariable IREmitter::multiply(IRValue lhs, IRValue rhs, bool set_flags) {
    auto dst = create_variable();
    push<IRMultiply>(dst, lhs, rhs, set_flags);
    return dst;
}

IRVariable IREmitter::add_carry(IRValue lhs, IRValue rhs, bool set_flags) {
    auto dst = create_variable();
    push<IRAddCarry>(dst, lhs, rhs, set_flags);
    return dst;
}

} // namespace arm