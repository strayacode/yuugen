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

    void save_host_regs();
    void restore_host_regs();

    CodeCache<JitFunction> code_cache;
    CodeBlock code_block;
    A64Assembler assembler;
    Jit& jit;
};

} // namespace arm