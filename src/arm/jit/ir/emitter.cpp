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

void Emitter::set_carry() {
    push<IRSetCarry>();
}

void Emitter::clear_carry() {
    push<IRClearCarry>();
}

IRVariable Emitter::move(IRValue src, bool set_flags) {
    auto dst = create_variable();
    push<IRMove>(dst, src, set_flags);
    return dst;
}

void Emitter::advance_pc() {
    // TODO: could this potentially be optimised by just storing the constant code_address + 12?
    auto pc = load_gpr(GPR::PC);
    auto updated_pc = add(pc, IRConstant{basic_block.location.get_instruction_size()}, false);
    store_gpr(GPR::PC, updated_pc);
}

IRVariable Emitter::load_gpr(GPR gpr) {
    auto dst = create_variable();
    GuestRegister src{gpr, basic_block.location.get_mode()};
    push<IRLoadGPR>(dst, src);
    return dst;
}

void Emitter::store_gpr(GPR gpr, IRVariable src) {
    GuestRegister dst{gpr, basic_block.location.get_mode()};
    push<IRStoreGPR>(dst, src);
}

IRVariable Emitter::add(IRValue lhs, IRValue rhs, bool set_flags) {
    auto dst = create_variable();
    push<IRAdd>(dst, lhs, rhs, set_flags);
    return dst;
}

} // namespace arm