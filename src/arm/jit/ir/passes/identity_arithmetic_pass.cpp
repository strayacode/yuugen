#include "arm/jit/ir/passes/identity_arithmetic_pass.h"

namespace arm {

void IdentityArithmeticPass::optimise(BasicBlock& basic_block) {
    // convert loads to copies with last known value
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
    if (opcode.lhs.is_constant() && !opcode.rhs.is_constant()) {
        switch (opcode.lhs.as_constant().value) {
        case 0x00000000:
            opcode_variant = std::make_unique<IRCopy>(opcode.dst, opcode.lhs);
            break;
        case 0xffffffff:
            opcode_variant = std::make_unique<IRCopy>(opcode.dst, opcode.rhs);
            break;
        }
    } else if (!opcode.lhs.is_constant() && opcode.rhs.is_constant()) {
        switch (opcode.rhs.as_constant().value) {
        case 0x00000000:
            opcode_variant = std::make_unique<IRCopy>(opcode.dst, opcode.rhs);
            break;
        case 0xffffffff:
            opcode_variant = std::make_unique<IRCopy>(opcode.dst, opcode.lhs);
            break;
        }
    }
}

void IdentityArithmeticPass::identity_bitwise_or(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRBitwiseOr>();
    if (opcode.lhs.is_constant() && !opcode.rhs.is_constant()) {
        switch (opcode.lhs.as_constant().value) {
        case 0x00000000:
            opcode_variant = std::make_unique<IRCopy>(opcode.dst, opcode.rhs);
            break;
        case 0xffffffff:
            opcode_variant = std::make_unique<IRCopy>(opcode.dst, opcode.lhs);
            break;
        }
    } else if (!opcode.lhs.is_constant() && opcode.rhs.is_constant()) {
        switch (opcode.rhs.as_constant().value) {
        case 0x00000000:
            opcode_variant = std::make_unique<IRCopy>(opcode.dst, opcode.lhs);
            break;
        case 0xffffffff:
            opcode_variant = std::make_unique<IRCopy>(opcode.dst, opcode.rhs);
            break;
        }
    }
}

void IdentityArithmeticPass::identity_bitwise_exclusive_or(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRBitwiseExclusiveOr>();
    if (opcode.lhs.is_constant() && !opcode.rhs.is_constant()) {
        switch (opcode.lhs.as_constant().value) {
        case 0x00000000:
            opcode_variant = std::make_unique<IRCopy>(opcode.dst, opcode.rhs);
            break;
        }
        
    } else if (!opcode.lhs.is_constant() && opcode.rhs.is_constant()) {
        switch (opcode.rhs.as_constant().value) {
        case 0x00000000:
            opcode_variant = std::make_unique<IRCopy>(opcode.dst, opcode.lhs);
            break;
        }
    }
}

void IdentityArithmeticPass::identity_logical_shift_left(std::unique_ptr<IROpcode>& opcode_variant) {

}

void IdentityArithmeticPass::identity_logical_shift_right(std::unique_ptr<IROpcode>& opcode_variant) {

}

void IdentityArithmeticPass::identity_arithmetic_shift_right(std::unique_ptr<IROpcode>& opcode_variant) {

}

void IdentityArithmeticPass::identity_rotate_right(std::unique_ptr<IROpcode>& opcode_variant) {

}

void IdentityArithmeticPass::identity_add(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRAdd>();
    if (opcode.lhs.is_constant() && !opcode.rhs.is_constant()) {
        switch (opcode.lhs.as_constant().value) {
        case 0x00000000:
            opcode_variant = std::make_unique<IRCopy>(opcode.dst, opcode.rhs);
            break;
        }
        
    } else if (!opcode.lhs.is_constant() && opcode.rhs.is_constant()) {
        switch (opcode.rhs.as_constant().value) {
        case 0x00000000:
            opcode_variant = std::make_unique<IRCopy>(opcode.dst, opcode.lhs);
            break;
        }
    }
}

void IdentityArithmeticPass::identity_subtract(std::unique_ptr<IROpcode>& opcode_variant) {
    auto& opcode = *opcode_variant->as<IRSubtract>();
    if (opcode.lhs.is_constant() && !opcode.rhs.is_constant()) {
        switch (opcode.lhs.as_constant().value) {
        case 0x00000000:
            opcode_variant = std::make_unique<IRCopy>(opcode.dst, opcode.rhs);
            break;
        }
        
    } else if (!opcode.lhs.is_constant() && opcode.rhs.is_constant()) {
        switch (opcode.rhs.as_constant().value) {
        case 0x00000000:
            opcode_variant = std::make_unique<IRCopy>(opcode.dst, opcode.lhs);
            break;
        }
    }
}

void IdentityArithmeticPass::identity_multiply(std::unique_ptr<IROpcode>& opcode_variant) {

}

} // namespace arm