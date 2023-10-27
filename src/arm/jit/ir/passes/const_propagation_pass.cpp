#include "common/logger.h"
#include "arm/jit/ir/passes/const_propagation_pass.h"

namespace arm {

void ConstPropagationPass::optimise(BasicBlock& basic_block) {
    // for (auto& opcode : basic_block.opcodes) {
    //     switch (opcode->get_type()) {
    //     case IROpcodeType::BitwiseAnd:
    //         logger.warn("handle bitwise and");
    //         break;
    //     case IROpcodeType::BitwiseOr:
    //         logger.warn("handle bitwise or");
    //         break;
    //     case IROpcodeType::BitwiseNot:
    //         handle_bitwise_not(opcode);
    //         break;
    //     case IROpcodeType::BitwiseExclusiveOr:
    //         logger.warn("handle bitwise exclusive or");
    //         break;
    //     case IROpcodeType::LogicalShiftLeft:
    //         logger.warn("handle bitwise lsl");
    //         break;
    //     case IROpcodeType::LogicalShiftRight:
    //         logger.warn("handle bitwise lsr");
    //         break;
    //     case IROpcodeType::ArithmeticShiftRight:
    //         logger.warn("handle bitwise asr");
    //         break;
    //     case IROpcodeType::CountLeadingZeroes:
    //         logger.warn("handle bitwise clz");
    //         break;
    //     case IROpcodeType::Add:
    //         handle_add(opcode);
    //         break;
    //     case IROpcodeType::AddLong:
    //         logger.warn("handle add long");
    //         break;
    //     case IROpcodeType::Subtract:
    //         logger.warn("handle subtract");
    //         break;
    //     case IROpcodeType::Multiply:
    //         logger.warn("handle multiply");
    //         break;
    //     case IROpcodeType::MultiplyLong:
    //         logger.warn("handle multiply long");
    //         break;
    //     default:
    //         break;
    //     }
    // }
}

void ConstPropagationPass::handle_add(std::unique_ptr<IROpcode>& opcode) {
    logger.warn("handle add");
    auto& add_opcode = *opcode->as<IRAdd>();
}

void ConstPropagationPass::handle_bitwise_not(std::unique_ptr<IROpcode>& opcode) {
    auto& bitwise_not_opcode = *opcode->as<IRBitwiseNot>();
    if (bitwise_not_opcode.src.is_constant()) {
        const u32 src = bitwise_not_opcode.src.as_constant().value;
        IRConstant constant{~src};
        opcode = std::make_unique<IRCopy>(bitwise_not_opcode.dst, constant);
    }
}

} // namespace arm