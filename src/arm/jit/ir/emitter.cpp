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

void Emitter::move(IRValue src, bool set_flags) {
    auto dst = create_variable();
    push<IRMove>(dst, src, set_flags);
}

} // namespace arm