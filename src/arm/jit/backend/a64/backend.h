#pragma once

#include "common/logger.h"
#include "arm/jit/basic_block.h"
#include "arm/jit/backend/backend.h"
#include "arm/jit/backend/code_cache.h"
#include "arm/jit/backend/a64/code_block.h"
#include "arm/jit/backend/a64/assembler.h"

namespace arm {

class Jit;

class A64Backend : public Backend {
public:
    A64Backend(Jit& jit);

    void reset() override;
    bool has_code_at(Location location) override;
    void compile(BasicBlock& basic_block) override;
    int run(Location location) override;

private:
    using JitFunction = void (*)();

    void compile_prologue();
    void compile_epilogue();

    CodeCache<JitFunction> code_cache;
    CodeBlock code_block;
    A64Assembler assembler;
    Jit& jit;

    static constexpr int CODE_CACHE_SIZE = 16 * 1024 * 1024;

    // we have x19, x20, x21, x22, x23, x24, x25, x26, x27 and x28 available for register allocation
};

} // namespace arm