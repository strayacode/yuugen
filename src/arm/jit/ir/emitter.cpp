#include "arm/jit/ir/opcodes.h"
#include "arm/jit/ir/emitter.h"

namespace arm {

Emitter::Emitter(BasicBlock& basic_block) : basic_block(basic_block) {}

void Emitter::set_carry() {
    push<IRSetCarry>();
}

void Emitter::clear_carry() {
    push<IRClearCarry>();
}

} // namespace arm