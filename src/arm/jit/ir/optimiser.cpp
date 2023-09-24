#include "arm/jit/ir/optimiser.h"

void Optimiser::optimise(BasicBlock& basic_block) {
    for (auto& pass : passes) {
        pass->optimise(basic_block);
    }
}

void Optimiser::add_pass(std::unique_ptr<Pass> pass) {
    passes.push_back(pass);
}