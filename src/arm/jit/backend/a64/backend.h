#pragma once

#include "common/logger.h"
#include "arm/state.h"
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
    int run(Location location, int cycles_left) override;

private:
    // return value: the cycles left after running the jit function (w0)
    // argument 1: a 64-bit pointer to the cpu state (x0)
    // argument 2: the cycles left (w1)
    using JitFunction = int (*)(State* state, int cycles_left);

    void compile_prologue();
    void compile_epilogue();

    void compile_ir_opcode(std::unique_ptr<IROpcode>& opcode);

    CodeCache<JitFunction> code_cache;
    CodeBlock code_block;
    A64Assembler assembler;
    Jit& jit;

    static constexpr int CODE_CACHE_SIZE = 16 * 1024 * 1024;

    static constexpr XReg state_reg = x19;
    static constexpr WReg cycles_left_reg = w20;

    // we have x21, x22, x23, x24, x25, x26, x27 and x28 available for register allocation
};

} // namespace arm