#include "common/logger.h"
#include "arm/jit/ir/passes/const_propagation_pass.h"

namespace arm {

void ConstPropagationPass::optimise(BasicBlock& basic_block) {
    record_uses(basic_block);

    auto it = basic_block.opcodes.begin();
    while (it != basic_block.opcodes.end()) {
        auto fold_result = fold_opcode(*it);
        bool erase_opcode = false;
        if (fold_result) {
            for (auto& use : uses) {
                if (use->is_variable() && use->as_variable().id == fold_result->dst_id) {
                    *use = IRConstant{fold_result->folded_value};
                    erase_opcode = true;
                }
            }
        }

        if (erase_opcode) {
            it = basic_block.opcodes.erase(it);
            mark_modified();
        } else {
            it++;
        }
    }
}

void ConstPropagationPass::record_uses(BasicBlock& basic_block) {
    uses.clear();

    auto it = basic_block.opcodes.rbegin();
    auto end = basic_block.opcodes.rend();

    while (it != end) {
        auto& opcode = *it;
        auto parameters = opcode->get_parameters();
        for (auto& parameter : parameters) {
            if (parameter->is_variable()) {
                uses.push_back(parameter);
            }
        }

        it++;
    }
}

std::optional<ConstPropagationPass::FoldResult> ConstPropagationPass::fold_opcode(std::unique_ptr<IROpcode>& opcode_variant) {
    // TODO: handle rotate right and barrel shifter opcodes
    switch (opcode_variant->get_type()) {
    case IROpcodeType::BitwiseAnd:
        return fold_bitwise_and(opcode_variant);
    case IROpcodeType::BitwiseOr:
        return fold_bitwise_or(opcode_variant);
    case IROpcodeType::BitwiseNot:
        return fold_bitwise_not(opcode_variant);
    case IROpcodeType::BitwiseExclusiveOr:
        return fold_bitwise_exclusive_or(opcode_variant);
    case IROpcodeType::LogicalShiftLeft:
        return fold_logical_shift_left(opcode_variant);
    case IROpcodeType::LogicalShiftRight:
        return fold_logical_shift_right(opcode_variant);
    case IROpcodeType::ArithmeticShiftRight:
        return fold_arithmetic_shift_right(opcode_variant);
    case IROpcodeType::CountLeadingZeroes:
        return fold_count_leading_zeroes(opcode_variant);
    case IROpcodeType::Add:
        return fold_add(opcode_variant);
    case IROpcodeType::AddLong:
        return fold_add_long(opcode_variant);
    case IROpcodeType::Subtract:
        return fold_subtract(opcode_variant);
    case IROpcodeType::Multiply:
        return fold_multiply(opcode_variant);
    case IROpcodeType::MultiplyLong:
        return fold_multiply_long(opcode_variant);
    case IROpcodeType::Compare:
        return fold_compare(opcode_variant);
    case IROpcodeType::Copy:
        return fold_copy(opcode_variant);
    default:
        return std::nullopt;
    }
}

std::optional<ConstPropagationPass::FoldResult> ConstPropagationPass::fold_bitwise_and(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRBitwiseAnd>();
    if (!opcode.lhs.is_constant() || !opcode.rhs.is_constant()) {
        return std::nullopt;
    }

    const auto& lhs = opcode.lhs.as_constant();
    const auto& rhs = opcode.rhs.as_constant();
    return FoldResult{lhs.value & rhs.value, opcode.dst.as_variable().id};
}

std::optional<ConstPropagationPass::FoldResult> ConstPropagationPass::fold_bitwise_or(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRBitwiseOr>();
    if (!opcode.lhs.is_constant() || !opcode.rhs.is_constant()) {
        return std::nullopt;
    }

    const auto& lhs = opcode.lhs.as_constant();
    const auto& rhs = opcode.rhs.as_constant();
    return FoldResult{lhs.value | rhs.value, opcode.dst.as_variable().id};
}

std::optional<ConstPropagationPass::FoldResult> ConstPropagationPass::fold_bitwise_not(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRBitwiseNot>();
    if (!opcode.src.is_constant()) {
        return std::nullopt;
    }

    const auto& src = opcode.src.as_constant();
    return FoldResult{~src.value, opcode.dst.as_variable().id};
}

std::optional<ConstPropagationPass::FoldResult> ConstPropagationPass::fold_bitwise_exclusive_or(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRBitwiseExclusiveOr>();
    if (!opcode.lhs.is_constant() || !opcode.rhs.is_constant()) {
        return std::nullopt;
    }

    const auto& lhs = opcode.lhs.as_constant();
    const auto& rhs = opcode.rhs.as_constant();
    return FoldResult{lhs.value ^ rhs.value, opcode.dst.as_variable().id};
}

std::optional<ConstPropagationPass::FoldResult> ConstPropagationPass::fold_logical_shift_left(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRLogicalShiftLeft>();
    if (!opcode.src.is_constant() || !opcode.amount.is_constant()) {
        return std::nullopt;
    }

    const auto& src = opcode.src.as_constant();
    const auto& amount = opcode.amount.as_constant();
    return FoldResult{src.value << amount.value, opcode.dst.as_variable().id};
}

std::optional<ConstPropagationPass::FoldResult> ConstPropagationPass::fold_logical_shift_right(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRLogicalShiftRight>();
    if (!opcode.src.is_constant() || !opcode.amount.is_constant()) {
        return std::nullopt;
    }

    const auto& src = opcode.src.as_constant();
    const auto& amount = opcode.amount.as_constant();
    return FoldResult{src.value >> amount.value, opcode.dst.as_variable().id};
}

std::optional<ConstPropagationPass::FoldResult> ConstPropagationPass::fold_arithmetic_shift_right(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRArithmeticShiftRight>();
    if (!opcode.src.is_constant() || !opcode.amount.is_constant()) {
        return std::nullopt;
    }

    const auto& src = opcode.src.as_constant();
    const auto& amount = opcode.amount.as_constant();
    return FoldResult{static_cast<u32>(static_cast<s32>(src.value) >> amount.value), opcode.dst.as_variable().id};
}

std::optional<ConstPropagationPass::FoldResult> ConstPropagationPass::fold_count_leading_zeroes(std::unique_ptr<IROpcode>& opcode_variant) {
    LOG_WARN("handle opcode count_leading_zeroes");
    auto& opcode = *opcode_variant->as<IRCountLeadingZeroes>();
    return std::nullopt;
}

std::optional<ConstPropagationPass::FoldResult> ConstPropagationPass::fold_add(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRAdd>();
    if (!opcode.lhs.is_constant() || !opcode.rhs.is_constant()) {
        return std::nullopt;
    }

    const auto& lhs = opcode.lhs.as_constant();
    const auto& rhs = opcode.rhs.as_constant();
    return FoldResult{lhs.value + rhs.value, opcode.dst.as_variable().id};
}

std::optional<ConstPropagationPass::FoldResult> ConstPropagationPass::fold_add_long(std::unique_ptr<IROpcode>& opcode_variant) {
    LOG_WARN("handle opcode add_long");
    auto& opcode = *opcode_variant->as<IRAddLong>();
    return std::nullopt;
}

std::optional<ConstPropagationPass::FoldResult> ConstPropagationPass::fold_subtract(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRSubtract>();
    if (!opcode.lhs.is_constant() || !opcode.rhs.is_constant()) {
        return std::nullopt;
    }

    const auto& lhs = opcode.lhs.as_constant();
    const auto& rhs = opcode.rhs.as_constant();
    return FoldResult{lhs.value - rhs.value, opcode.dst.as_variable().id};
}

std::optional<ConstPropagationPass::FoldResult> ConstPropagationPass::fold_multiply(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRMultiply>();
    if (!opcode.lhs.is_constant() || !opcode.rhs.is_constant()) {
        return std::nullopt;
    }

    const auto& lhs = opcode.lhs.as_constant();
    const auto& rhs = opcode.rhs.as_constant();
    return FoldResult{lhs.value * rhs.value, opcode.dst.as_variable().id};
}

std::optional<ConstPropagationPass::FoldResult> ConstPropagationPass::fold_multiply_long(std::unique_ptr<IROpcode>& opcode_variant) {
    LOG_WARN("handle opcode multiply_long");
    auto& opcode = *opcode_variant->as<IRMultiplyLong>();
    return std::nullopt;
}

std::optional<ConstPropagationPass::FoldResult> ConstPropagationPass::fold_compare(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRCompare>();
    const auto& dst = opcode.dst.as_variable();
    if (!opcode.lhs.is_constant() || !opcode.rhs.is_constant()) {
        return std::nullopt;
    }

    const auto& lhs = opcode.lhs.as_constant();
    const auto& rhs = opcode.rhs.as_constant();
    switch (opcode.compare_type) {
    case CompareType::Equal:
        return FoldResult{lhs.value == rhs.value, dst.id};
    case CompareType::LessThan:
        return FoldResult{lhs.value < rhs.value, dst.id};
    case CompareType::GreaterEqual:
        return FoldResult{lhs.value >= rhs.value, dst.id};
    case CompareType::GreaterThan:
        return FoldResult{lhs.value > rhs.value, dst.id};
    }
}

std::optional<ConstPropagationPass::FoldResult> ConstPropagationPass::fold_copy(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRCopy>();
    if (!opcode.src.is_constant()) {
        return std::nullopt;
    }

    const auto& dst = opcode.dst.as_variable();
    const auto& src = opcode.src.as_constant();
    return FoldResult{src.value, dst.id};
}

} // namespace arm