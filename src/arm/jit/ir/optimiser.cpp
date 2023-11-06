#include "common/logger.h"
#include "arm/jit/ir/optimiser.h"

namespace arm {

void Optimiser::optimise(BasicBlock& basic_block) {
    int max_iterations = 5;
    for (int i = 0; i < max_iterations; i++) {
        bool modified = false;

        for (auto& pass : passes) {
            pass->clear_modified();
            pass->optimise(basic_block);
            modified |= pass->modified_basic_block();
        }

        if (!modified) {
            break;
        }
    }
}

void Optimiser::add_pass(std::unique_ptr<Pass> pass) {
    passes.push_back(std::move(pass));
}

} // namespace arm