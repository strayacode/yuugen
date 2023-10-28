#include "arm/jit/backend/a64/backend.h"
#include "arm/jit/jit.h"

namespace arm {

A64Backend::A64Backend(Jit& jit) : jit(jit) {}

void A64Backend::reset() {
    assembler.reset();
}

bool A64Backend::has_code_at(Location location) {
    return code_cache.has_code_at(location);
}

void A64Backend::compile(BasicBlock& basic_block) {
    assembler.reset();
    assembler.ret();

    assembler.dump();
    logger.todo("handle end of compilation");
}

int A64Backend::run(Location location) {
    logger.todo("run");
    return 0;
}

} // namespace arm