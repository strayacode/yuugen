#include "arm/jit/translator.h"
#include "arm/jit/jit.h"

namespace arm {

Translator::Translator(Jit& jit) : jit(jit) {}

void Translator::translate(BasicBlock& basic_block) {
    logger.todo("Translator: translate basic block");
}

} // namespace arm