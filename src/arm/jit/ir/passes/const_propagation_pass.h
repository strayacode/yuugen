#pragma once

#include <vector>
#include <memory>
#include "arm/jit/basic_block.h"
#include "arm/jit/ir/value.h"
#include "arm/jit/ir/pass.h"

namespace arm {

class ConstPropagationPass : public Pass {
public:
    void optimise(BasicBlock& basic_block) override;

private:
    void handle_add(std::unique_ptr<IROpcode>& opcode);
    void handle_bitwise_not(std::unique_ptr<IROpcode>& opcode);

    std::vector<IRVariable*> uses;
};

} // namespace arm