#include "arm/jit/ir/emitter.h"

namespace arm {

Emitter::Emitter(BasicBlock& basic_block) : basic_block(basic_block) {}

} // namespace arm