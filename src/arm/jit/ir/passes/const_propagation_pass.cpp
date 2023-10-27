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
                    logger.debug("folded %s into uses of v%d", (*it)->to_string().c_str(), fold_result->dst_id);
                    *use = IRConstant{fold_result->folded_value};
                    erase_opcode = true;
                }
            }
        }

        if (erase_opcode) {
            it = basic_block.opcodes.erase(it);
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
    switch (opcode_variant->get_type()) {
    case IROpcodeType::BitwiseAnd:
        logger.warn("handle bitwise and");
        return std::nullopt;
    case IROpcodeType::BitwiseOr:
        logger.warn("handle bitwise or");
        return std::nullopt;
    case IROpcodeType::BitwiseNot:
        return fold_bitwise_not(opcode_variant);
    case IROpcodeType::BitwiseExclusiveOr:
        logger.warn("handle bitwise exclusive or");
        return std::nullopt;
    case IROpcodeType::LogicalShiftLeft:
        logger.warn("handle bitwise lsl");
        return std::nullopt;
    case IROpcodeType::LogicalShiftRight:
        logger.warn("handle bitwise lsr");
        return std::nullopt;
    case IROpcodeType::ArithmeticShiftRight:
        logger.warn("handle bitwise asr");
        return std::nullopt;
    case IROpcodeType::CountLeadingZeroes:
        logger.warn("handle bitwise clz");
        return std::nullopt;
    case IROpcodeType::Add:
        return fold_add(opcode_variant);
    case IROpcodeType::AddLong:
        logger.warn("handle add long");
        return std::nullopt;
    case IROpcodeType::Subtract:
        logger.warn("handle subtract");
        return std::nullopt;
    case IROpcodeType::Multiply:
        logger.warn("handle multiply");
        return std::nullopt;
    case IROpcodeType::MultiplyLong:
        logger.warn("handle multiply long");
        return std::nullopt;
    case IROpcodeType::Copy:
        return fold_copy(opcode_variant);
    default:
        return std::nullopt;
    }
}

std::optional<ConstPropagationPass::FoldResult> ConstPropagationPass::fold_add(std::unique_ptr<IROpcode>& opcode_variant) {
    logger.warn("handle add");
    auto& add_opcode = *opcode_variant->as<IRAdd>();
    return std::nullopt;
}

std::optional<ConstPropagationPass::FoldResult> ConstPropagationPass::fold_bitwise_not(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRBitwiseNot>();
    if (!opcode.src.is_constant()) {
        return std::nullopt;
    }

    const auto& src = opcode.src.as_constant();
    return FoldResult{~src.value, opcode.dst.id};
}

std::optional<ConstPropagationPass::FoldResult> ConstPropagationPass::fold_copy(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRCopy>();
    if (!opcode.src.is_constant()) {
        return std::nullopt;
    }

    const auto& src = opcode.src.as_constant();
    return FoldResult{src.value, opcode.dst.id};
}

} // namespace arm