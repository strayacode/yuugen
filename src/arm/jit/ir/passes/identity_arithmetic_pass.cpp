#include "arm/jit/ir/passes/identity_arithmetic_pass.h"

namespace arm {

void IdentityArithmeticPass::optimise(BasicBlock& basic_block) {
    for (auto& opcode : basic_block.opcodes) {
        identity_opcode(opcode);
    }
}

void IdentityArithmeticPass::identity_opcode(std::unique_ptr<IROpcode>& opcode_variant) {
    switch (opcode_variant->get_type()) {
    case IROpcodeType::BitwiseAnd:
        identity_bitwise_and(opcode_variant);
        break;
    case IROpcodeType::BitwiseOr:
        identity_bitwise_or(opcode_variant);
        break;
    case IROpcodeType::BitwiseExclusiveOr:
        identity_bitwise_exclusive_or(opcode_variant);
        break;
    case IROpcodeType::LogicalShiftLeft:
        identity_logical_shift_left(opcode_variant);
        break;
    case IROpcodeType::LogicalShiftRight:
        identity_logical_shift_right(opcode_variant);
        break;
    case IROpcodeType::ArithmeticShiftRight:
        identity_arithmetic_shift_right(opcode_variant);
        break;
    case IROpcodeType::RotateRight:
        identity_rotate_right(opcode_variant);
        break;
    case IROpcodeType::Add:
        identity_add(opcode_variant);
        break;
    case IROpcodeType::Subtract:
        identity_subtract(opcode_variant);
        break;
    case IROpcodeType::Multiply:
        identity_multiply(opcode_variant);
        break;
    default:
        break;
    }
}

void IdentityArithmeticPass::identity_bitwise_and(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRBitwiseAnd>();
    auto lhs = opcode.lhs;
    auto rhs = opcode.rhs;

    if (lhs.is_equal(0) || rhs.is_equal(0)) {
        opcode_variant = std::make_unique<IRCopy>(opcode.dst, IRConstant{0});
    } else if (lhs.is_equal(0xffffffff)) {
        opcode_variant = std::make_unique<IRCopy>(opcode.dst, rhs);
    } else if (rhs.is_equal(0xffffffff)) {
        opcode_variant = std::make_unique<IRCopy>(opcode.dst, lhs);
    }
}

void IdentityArithmeticPass::identity_bitwise_or(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRBitwiseOr>();
    auto lhs = opcode.lhs;
    auto rhs = opcode.rhs;

    if (lhs.is_equal(0xffffffff) || rhs.is_equal(0xffffffff)) {
        opcode_variant = std::make_unique<IRCopy>(opcode.dst, IRConstant{0xffffffff});
    } else if (lhs.is_equal(0)) {
        opcode_variant = std::make_unique<IRCopy>(opcode.dst, rhs);
    } else if (rhs.is_equal(0)) {
        opcode_variant = std::make_unique<IRCopy>(opcode.dst, lhs);
    }
}

void IdentityArithmeticPass::identity_bitwise_exclusive_or(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRBitwiseExclusiveOr>();
    auto lhs = opcode.lhs;
    auto rhs = opcode.rhs;

    if (lhs.is_equal(0)) {
        opcode_variant = std::make_unique<IRCopy>(opcode.dst, rhs);
    } else if (rhs.is_equal(0)) {
        opcode_variant = std::make_unique<IRCopy>(opcode.dst, lhs);
    }
}

void IdentityArithmeticPass::identity_logical_shift_left(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRLogicalShiftLeft>();
    auto src = opcode.src;
    auto amount = opcode.amount;

    if (amount.is_equal(0)) {
        opcode_variant = std::make_unique<IRCopy>(opcode.dst, src);
    }
}

void IdentityArithmeticPass::identity_logical_shift_right(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRLogicalShiftRight>();
    auto src = opcode.src;
    auto amount = opcode.amount;

    if (amount.is_equal(0)) {
        opcode_variant = std::make_unique<IRCopy>(opcode.dst, src);
    }
}

void IdentityArithmeticPass::identity_arithmetic_shift_right(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRArithmeticShiftRight>();
    auto src = opcode.src;
    auto amount = opcode.amount;

    if (amount.is_equal(0)) {
        opcode_variant = std::make_unique<IRCopy>(opcode.dst, src);
    }
}

void IdentityArithmeticPass::identity_rotate_right(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRRotateRight>();
    auto src = opcode.src;
    auto amount = opcode.amount;

    if (amount.is_equal(0)) {
        opcode_variant = std::make_unique<IRCopy>(opcode.dst, src);
    }
}

void IdentityArithmeticPass::identity_add(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRAdd>();
    auto lhs = opcode.lhs;
    auto rhs = opcode.rhs;

    if (lhs.is_equal(0)) {
        opcode_variant = std::make_unique<IRCopy>(opcode.dst, rhs);
    } else if (rhs.is_equal(0)) {
        opcode_variant = std::make_unique<IRCopy>(opcode.dst, lhs);
    }
}

void IdentityArithmeticPass::identity_subtract(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRSubtract>();
    auto lhs = opcode.lhs;
    auto rhs = opcode.rhs;

    if (rhs.is_equal(0)) {
        opcode_variant = std::make_unique<IRCopy>(opcode.dst, lhs);
    }
}

void IdentityArithmeticPass::identity_multiply(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRAdd>();
    auto lhs = opcode.lhs;
    auto rhs = opcode.rhs;

    if (lhs.is_equal(1)) {
        opcode_variant = std::make_unique<IRCopy>(opcode.dst, rhs);
    } else if (rhs.is_equal(1)) {
        opcode_variant = std::make_unique<IRCopy>(opcode.dst, lhs);
    }
}

} // namespace arm