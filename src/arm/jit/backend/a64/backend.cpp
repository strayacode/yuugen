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
    JitFunction fn = assembler.get_current_code<JitFunction>();
    code_block.unprotect();

    compile_prologue();
    compile_epilogue();

    code_block.protect();

    assembler.dump();
    logger.todo("handle end of compilation");
}

int A64Backend::run(Location location) {
    logger.todo("run");
    return 0;
}

void A64Backend::compile_prologue() {
    // save non-volatile registers to the stack
    assembler.stp(x19, x20, sp, IndexMode::Pre, -96);
    assembler.stp(x21, x22, sp, 16);
    assembler.stp(x23, x24, sp, 32);
    assembler.stp(x25, x26, sp, 48);
    assembler.stp(x27, x28, sp, 64);
    assembler.stp(x29, x30, sp, 80);

    // store the cpu state pointer into the pinned register
    assembler.mov(state_reg, x0);

    // store the cycles left into the cycles left pinned register
    assembler.mov(cycles_left_reg, w1);

    // TODO: store pointer of cpu state struct into a pinned register
    // TODO: use a dedicated pinned register to record how many cycles have elapsed
    // in the basic block

    // TODO: generate a condition check for when basic block
    // condition is not AL or NV

    // TODO: check interrupts in jitted code
    
    // x0 will contain a pointer to the cpu state struct
    // TODO: store cycles left in x1 eventually so when we do block linking,
    // we know when enough cycles has passed
}

void A64Backend::compile_epilogue() {
    // restore non-volatile registers from the stack
    assembler.ldp(x29, x30, sp, 80);
    assembler.ldp(x27, x28, sp, 64);
    assembler.ldp(x25, x26, sp, 48);
    assembler.ldp(x23, x24, sp, 32);
    assembler.ldp(x21, x22, sp, 16);
    assembler.ldp(x19, x20, sp, IndexMode::Post, 96);

    // store the cycles left into w0
    assembler.mov(w0, cycles_left_reg);
    assembler.ret();
}

} // namespace arm