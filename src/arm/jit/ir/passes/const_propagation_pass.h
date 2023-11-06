#pragma once

#include <vector>
#include <memory>
#include <optional>
#include "arm/jit/basic_block.h"
#include "arm/jit/ir/value.h"
#include "arm/jit/ir/pass.h"

namespace arm {

class ConstPropagationPass : public Pass {
public:
    void optimise(BasicBlock& basic_block) override;

private:
    void record_uses(BasicBlock& basic_block);

    struct FoldResult {
        u32 folded_value;
        u32 dst_id;
    };

    std::optional<FoldResult> fold_opcode(std::unique_ptr<IROpcode>& opcode_variant);
    std::optional<FoldResult> fold_bitwise_and(std::unique_ptr<IROpcode>& opcode_variant);
    std::optional<FoldResult> fold_bitwise_or(std::unique_ptr<IROpcode>& opcode_variant);
    std::optional<FoldResult> fold_bitwise_not(std::unique_ptr<IROpcode>& opcode_variant);
    std::optional<FoldResult> fold_bitwise_exclusive_or(std::unique_ptr<IROpcode>& opcode_variant);
    std::optional<FoldResult> fold_logical_shift_left(std::unique_ptr<IROpcode>& opcode_variant);
    std::optional<FoldResult> fold_logical_shift_right(std::unique_ptr<IROpcode>& opcode_variant);
    std::optional<FoldResult> fold_arithmetic_shift_right(std::unique_ptr<IROpcode>& opcode_variant);
    std::optional<FoldResult> fold_count_leading_zeroes(std::unique_ptr<IROpcode>& opcode_variant);
    std::optional<FoldResult> fold_add(std::unique_ptr<IROpcode>& opcode_variant);
    std::optional<FoldResult> fold_add_long(std::unique_ptr<IROpcode>& opcode_variant);
    std::optional<FoldResult> fold_subtract(std::unique_ptr<IROpcode>& opcode_variant);
    std::optional<FoldResult> fold_multiply(std::unique_ptr<IROpcode>& opcode_variant);
    std::optional<FoldResult> fold_multiply_long(std::unique_ptr<IROpcode>& opcode_variant);
    std::optional<FoldResult> fold_compare(std::unique_ptr<IROpcode>& opcode_variant);
    std::optional<FoldResult> fold_copy(std::unique_ptr<IROpcode>& opcode_variant);

    std::vector<IRValue*> uses;
};

} // namespace arm