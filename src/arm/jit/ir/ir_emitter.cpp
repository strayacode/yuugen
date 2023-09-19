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

IRVariable IREmitter::move(IRValue src, bool set_flags) {
    auto dst = create_variable();
    push<IRMove>(dst, src, set_flags);
    return dst;
}

IRVariable IREmitter::add(IRValue lhs, IRValue rhs, bool set_flags) {
    auto dst = create_variable();
    push<IRAdd>(dst, lhs, rhs, set_flags);
    return dst;
}

IRVariable IREmitter::logical_shift_left(IRValue src, IRValue amount, bool set_carry) {
    auto dst = create_variable();
    push<IRLogicalShiftLeft>(dst, src, amount, set_carry);
    return dst;
}

IRVariable IREmitter::and_(IRValue lhs, IRValue rhs, bool set_flags) {
    auto dst = create_variable();
    push<IRAnd>(dst, lhs, rhs, set_flags);
    return dst;
}

IRVariable IREmitter::logical_shift_right(IRValue src, IRValue amount, bool set_carry) {
    auto dst = create_variable();
    push<IRLogicalShiftRight>(dst, src, amount, set_carry);
    return dst;
}

void IREmitter::memory_write(IRValue addr, IRVariable src, AccessSize access_size, AccessType access_type) {
    push<IRMemoryWrite>(addr, src, access_size, access_type);
}

IRVariable IREmitter::sub(IRValue lhs, IRValue rhs, bool set_flags) {
    auto dst = create_variable();
    push<IRSub>(dst, lhs, rhs, set_flags);
    return dst;
}

void IREmitter::update_flag(Flags flag, bool value) {
    push<IRUpdateFlag>(flag, value);
}

void IREmitter::store_flags(Flags flags) {
    push<IRStoreFlags>(flags);
}

void IREmitter::compare(IRValue lhs, IRValue rhs) {
    push<IRCompare>(lhs, rhs);
}

IRVariable IREmitter::or_(IRValue lhs, IRValue rhs, bool set_flags) {
    auto dst = create_variable();
    push<IROr>(dst, lhs, rhs, set_flags);
    return dst;
}

IRVariable IREmitter::arithmetic_shift_right(IRValue src, IRValue amount, bool set_carry) {
    auto dst = create_variable();
    push<IRArithmeticShiftRight>(dst, src, amount, set_carry);
    return dst;
}

IRVariable IREmitter::rotate_right(IRValue src, IRValue amount, bool set_carry) {
    auto dst = create_variable();
    push<IRRotateRight>(dst, src, amount, set_carry);
    return dst;
}

IRVariable IREmitter::memory_read(IRValue addr, AccessSize access_size, AccessType access_type) {
    auto dst = create_variable();
    push<IRMemoryRead>(dst, addr, access_size, access_type);
    return dst;
}

IRVariable IREmitter::bic(IRValue lhs, IRValue rhs, bool set_flags) {
    auto dst = create_variable();
    push<IRBic>(dst, lhs, rhs, set_flags);
    return dst;
}

void IREmitter::branch(IRValue address, bool is_arm) {
    push<IRBranch>(address, is_arm);
}

void IREmitter::branch_exchange(IRValue address, ExchangeType exchange_type) {
    push<IRBranchExchange>(address, exchange_type);
}

IRVariable IREmitter::multiply(IRValue lhs, IRValue rhs, bool set_flags) {
    auto dst = create_variable();
    push<IRMultiply>(dst, lhs, rhs, set_flags);
    return dst;
}

IRVariable IREmitter::exclusive_or(IRValue lhs, IRValue rhs, bool set_flags) {
    auto dst = create_variable();
    push<IRExclusiveOr>(dst, lhs, rhs, set_flags);
    return dst;
}

void IREmitter::test(IRValue lhs, IRValue rhs) {
    push<IRTest>(lhs, rhs);
}

IRVariable IREmitter::add_carry(IRValue lhs, IRValue rhs, bool set_flags) {
    auto dst = create_variable();
    push<IRAddCarry>(dst, lhs, rhs, set_flags);
    return dst;
}

IRVariable IREmitter::move_negate(IRValue src, bool set_flags) {
    auto dst = create_variable();
    push<IRMoveNegate>(dst, src, set_flags);
    return dst;
}

void IREmitter::compare_negate(IRValue lhs, IRValue rhs) {
    push<IRCompareNegate>(lhs, rhs);
}

} // namespace arm