#include "common/logger.h"
#include "arm/jit/ir/opcodes.h"
#include "arm/jit/ir/emitter.h"

namespace arm {

Emitter::Emitter(BasicBlock& basic_block) : basic_block(basic_block) {}

IRVariable Emitter::create_variable() {
    IRVariable variable{next_variable_id};
    next_variable_id++;
    return variable;
}

IRVariable Emitter::move(IRValue src, bool set_flags) {
    auto dst = create_variable();
    push<IRMove>(dst, src, set_flags);
    return dst;
}

IRVariable Emitter::load_gpr(GPR gpr) {
    auto dst = create_variable();
    GuestRegister src{gpr, basic_block.location.get_mode()};
    push<IRLoadGPR>(dst, src);
    return dst;
}

void Emitter::store_gpr(GPR gpr, IRValue src) {
    GuestRegister dst{gpr, basic_block.location.get_mode()};
    push<IRStoreGPR>(dst, src);
}

IRVariable Emitter::add(IRValue lhs, IRValue rhs, bool set_flags) {
    auto dst = create_variable();
    push<IRAdd>(dst, lhs, rhs, set_flags);
    return dst;
}

IRVariable Emitter::logical_shift_left(IRValue src, IRValue amount, bool set_carry) {
    auto dst = create_variable();
    push<IRLogicalShiftLeft>(dst, src, amount, set_carry);
    return dst;
}

IRVariable Emitter::and_(IRValue lhs, IRValue rhs, bool set_flags) {
    auto dst = create_variable();
    push<IRAnd>(dst, lhs, rhs, set_flags);
    return dst;
}

IRVariable Emitter::logical_shift_right(IRValue src, IRValue amount, bool set_carry) {
    auto dst = create_variable();
    push<IRLogicalShiftRight>(dst, src, amount, set_carry);
    return dst;
}

void Emitter::memory_write(IRValue addr, IRVariable src, AccessSize access_size, AccessType access_type) {
    push<IRMemoryWrite>(addr, src, access_size, access_type);
}

IRVariable Emitter::sub(IRValue lhs, IRValue rhs, bool set_flags) {
    auto dst = create_variable();
    push<IRSub>(dst, lhs, rhs, set_flags);
    return dst;
}

void Emitter::update_flag(Flags flag, bool value) {
    push<IRUpdateFlag>(flag, value);
}

void Emitter::store_flags(Flags flags) {
    push<IRStoreFlags>(flags);
}

void Emitter::compare(IRValue lhs, IRValue rhs) {
    push<IRCompare>(lhs, rhs);
}

IRVariable Emitter::load_cpsr() {
    auto dst = create_variable();
    push<IRLoadCPSR>(dst);
    return dst;
}

IRVariable Emitter::load_spsr() {
    auto dst = create_variable();
    push<IRLoadSPSR>(dst);
    return dst;
}

IRVariable Emitter::or_(IRValue lhs, IRValue rhs, bool set_flags) {
    auto dst = create_variable();
    push<IROr>(dst, lhs, rhs, set_flags);
    return dst;
}

void Emitter::store_cpsr(IRVariable src) {
    push<IRStoreCPSR>(src);
}

void Emitter::store_spsr(IRVariable src) {
    push<IRStoreSPSR>(src);
}

IRVariable Emitter::arithmetic_shift_right(IRValue src, IRValue amount, bool set_carry) {
    auto dst = create_variable();
    push<IRArithmeticShiftRight>(dst, src, amount, set_carry);
    return dst;
}

IRVariable Emitter::rotate_right(IRValue src, IRValue amount, bool set_carry) {
    auto dst = create_variable();
    push<IRRotateRight>(dst, src, amount, set_carry);
    return dst;
}

IRVariable Emitter::memory_read(IRValue addr, AccessSize access_size, AccessType access_type) {
    auto dst = create_variable();
    push<IRMemoryRead>(dst, addr, access_size, access_type);
    return dst;
}

IRVariable Emitter::bic(IRValue lhs, IRValue rhs, bool set_flags) {
    auto dst = create_variable();
    push<IRBic>(dst, lhs, rhs, set_flags);
    return dst;
}

} // namespace arm