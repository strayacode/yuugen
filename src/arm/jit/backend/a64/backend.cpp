#include "arm/jit/backend/a64/backend.h"
#include "arm/jit/jit.h"

namespace arm {

A64Backend::A64Backend(Jit& jit) : code_block(CODE_CACHE_SIZE), assembler(code_block.get_code()), jit(jit) {}

void A64Backend::reset() {
    
}

bool A64Backend::has_code_at(Location location) {
    return code_cache.has_code_at(location);
}

void A64Backend::compile(BasicBlock& basic_block) {
    code_block.unprotect();

    // save non-volatile registers to the stack
    save_host_regs();

    // restore non-volatile registers from the stack
    restore_host_regs();

    assembler.ret();

    code_block.protect();

    assembler.dump();
    logger.todo("handle end of compilation");
}

int A64Backend::run(Location location) {
    logger.todo("run");
    return 0;
}

void A64Backend::save_host_regs() {
    assembler.stp(x19, x20, sp, IndexMode::Pre, -96);
    assembler.stp(x21, x22, sp, 16);
    assembler.stp(x23, x24, sp, 32);
    assembler.stp(x25, x26, sp, 48);
    assembler.stp(x27, x28, sp, 64);
    assembler.stp(x29, x30, sp, 80);
}

void A64Backend::restore_host_regs() {
    // assembler.ldp()
}

} // namespace arm