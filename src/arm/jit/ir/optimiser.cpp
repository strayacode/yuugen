#include "arm/jit/ir/optimiser.h"

namespace arm {

void Optimiser::optimise(BasicBlock& basic_block) {
    logger.debug("before optimisations...");
    basic_block.dump();
    int i = 0;

    for (auto& pass : passes) {
        pass->optimise(basic_block);
        logger.debug("after optimisation #%d", i);
        basic_block.dump();
        i++;
    }
}

void Optimiser::add_pass(std::unique_ptr<Pass> pass) {
    passes.push_back(std::move(pass));
}

} // namespace arm