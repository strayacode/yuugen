#include "arm/jit/ir/optimiser.h"
// #include "arm/jit/ir/passes/dead_load_store_elimination_pass.h"

namespace arm {

void Optimiser::optimise(BasicBlock& basic_block) {
    for (auto& pass : passes) {
        pass->optimise(basic_block);
    }
}

void Optimiser::add_pass(std::unique_ptr<Pass> pass) {
    logger.todo("add pass");
}

} // namespace arm