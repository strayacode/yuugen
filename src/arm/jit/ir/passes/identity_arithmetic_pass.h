#pragma once

#include <array>
#include <memory>
#include "arm/jit/basic_block.h"
#include "arm/jit/ir/pass.h"

namespace arm {

class IdentityArithmeticPass : public Pass {
public:
    void optimise(BasicBlock& basic_block) override;

private:
    void identity_opcode(std::unique_ptr<IROpcode>& opcode_variant);
    void identity_bitwise_and(std::unique_ptr<IROpcode>& opcode_variant);
    void identity_bitwise_or(std::unique_ptr<IROpcode>& opcode_variant);
    void identity_bitwise_exclusive_or(std::unique_ptr<IROpcode>& opcode_variant);
    void identity_logical_shift_left(std::unique_ptr<IROpcode>& opcode_variant);
    void identity_logical_shift_right(std::unique_ptr<IROpcode>& opcode_variant);
    void identity_arithmetic_shift_right(std::unique_ptr<IROpcode>& opcode_variant);
    void identity_rotate_right(std::unique_ptr<IROpcode>& opcode_variant);
    void identity_add(std::unique_ptr<IROpcode>& opcode_variant);
    void identity_subtract(std::unique_ptr<IROpcode>& opcode_variant);
    void identity_multiply(std::unique_ptr<IROpcode>& opcode_variant);
};

} // namespace arm