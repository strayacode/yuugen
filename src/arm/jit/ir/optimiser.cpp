#include "common/logger.h"
#include "arm/jit/ir/optimiser.h"

namespace arm {

void Optimiser::optimise(BasicBlock& basic_block) {
    logger.debug("unoptimised");
    basic_block.dump();
    for (auto& pass : passes) {
        pass->optimise(basic_block);
    }
}

void Optimiser::add_pass(std::unique_ptr<Pass> pass) {
    passes.push_back(std::move(pass));
}

} // namespace arm