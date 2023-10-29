#include "arm/jit/backend/a64/backend.h"
#include "arm/jit/jit.h"

namespace arm {

A64Backend::A64Backend(Jit& jit) : code_block(1024 * 1024), assembler(code_block.get_code()), jit(jit) {}

void A64Backend::reset() {
    
}

bool A64Backend::has_code_at(Location location) {
    return code_cache.has_code_at(location);
}

void A64Backend::compile(BasicBlock& basic_block) {
    save_host_regs();

    // save non-volatile registers to the stack
    assembler.ret();

    restore_host_regs();

    assembler.dump();
    logger.todo("handle end of compilation");
}

int A64Backend::run(Location location) {
    logger.todo("run");
    return 0;
}

void A64Backend::save_host_regs() {

}

void A64Backend::restore_host_regs() {

}

} // namespace arm