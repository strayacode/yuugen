#include "arm/jit/ir/optimiser.h"

namespace arm {

void Optimiser::optimise(BasicBlock& basic_block) {
    for (auto& pass : passes) {
        pass->optimise(basic_block);
    }

    // logger.debug("after optimisations...");
    // basic_block.dump();
}

void Optimiser::add_pass(std::unique_ptr<Pass> pass) {
    passes.push_back(std::move(pass));
}

} // namespace arm