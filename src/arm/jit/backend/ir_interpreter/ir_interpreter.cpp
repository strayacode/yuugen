#include <cassert>
#include "common/logger.h"
#include "arm/jit/jit.h"
#include "arm/jit/backend/ir_interpreter/ir_interpreter.h"

namespace arm {

IRInterpreter::IRInterpreter(Jit& jit) : jit(jit) {}

void IRInterpreter::reset() {
    code_cache.reset();
    variables.clear();
}

bool IRInterpreter::has_code_at(Location location) {
    return code_cache.has_code_at(location);
}

void IRInterpreter::compile(BasicBlock& basic_block)  {
    CompiledBlock compiled_block;
    compiled_block.cycles = basic_block.cycles;
    compiled_block.num_instructions = basic_block.num_instructions;
    compiled_block.condition = basic_block.condition;
    compiled_block.location = basic_block.location;
    
    for (auto& opcode : basic_block.opcodes) {
        compiled_block.instructions.push_back(compile_ir_opcode(opcode));
    }

    code_cache.set(basic_block.location, std::move(compiled_block));
}

int IRInterpreter::run(Location location)  {
    auto& compiled_block = code_cache.get(location);
    if (evaluate_condition(compiled_block.condition)) {
        for (auto& compiled_instruction : compiled_block.instructions) {
            (this->*compiled_instruction.fn)(compiled_instruction.opcode_variant);
        }

        return compiled_block.cycles;
    } else {
        u32 pc_after_block = jit.get_gpr(GPR::PC) + ((compiled_block.num_instructions - 2) * compiled_block.location.get_instruction_size());
        jit.set_gpr(GPR::PC, pc_after_block);

        // when all instructions fail the condition check,
        // then each one takes 1 cycle (for fetching the instruction)
        return compiled_block.num_instructions;
    }
}

bool IRInterpreter::evaluate_condition(Condition condition) {
    bool n = flags & 8;
    bool z = flags & 4;
    bool c = flags & 2;
    bool v = flags & 1;

    switch (condition) {
    case Condition::EQ:
        return z;
    case Condition::NE:
        return !z;
    case Condition::CS:
        return c;
    case Condition::CC:
        return !c;
    case Condition::MI:
        return n;
    case Condition::PL:
        return !n;
    case Condition::VS:
        return v;
    case Condition::VC:
        return !v;
    case Condition::HI:
        return c && !z;
    case Condition::LS:
        return !c || z;
    case Condition::GE:
        return n == v;
    case Condition::LT:
        return n != v;
    case Condition::GT:
        return !z && (n == v);
    case Condition::LE:
        return z || (n != v);
    case Condition::AL:
        return true;
    case Condition::NV:
        return false;
    }
}

IRInterpreter::CompiledInstruction IRInterpreter::compile_ir_opcode(std::unique_ptr<IROpcode>& opcode) {
    auto type = opcode->get_type();
    switch (type) {
    case IROpcodeType::SetCarry:
        return {&IRInterpreter::handle_set_carry, *opcode->as<IRSetCarry>()};
    case IROpcodeType::ClearCarry:
        return {&IRInterpreter::handle_clear_carry, *opcode->as<IRClearCarry>()};
    case IROpcodeType::Move:
        return {&IRInterpreter::handle_move, *opcode->as<IRMove>()};
    case IROpcodeType::LoadGPR:
        return {&IRInterpreter::handle_load_gpr, *opcode->as<IRLoadGPR>()};
    case IROpcodeType::StoreGPR:
        return {&IRInterpreter::handle_store_gpr, *opcode->as<IRStoreGPR>()};
    case IROpcodeType::Add:
        return {&IRInterpreter::handle_add, *opcode->as<IRAdd>()};
    case IROpcodeType::LogicalShiftLeft:
        return {&IRInterpreter::handle_logical_shift_left, *opcode->as<IRLogicalShiftLeft>()};
    case IROpcodeType::And:
        return {&IRInterpreter::handle_and, *opcode->as<IRAnd>()};
    case IROpcodeType::LogicalShiftRight:
        return {&IRInterpreter::handle_logical_shift_right, *opcode->as<IRLogicalShiftRight>()};
    }
}

u32& IRInterpreter::get(IRVariable& variable) {
    assert(variables.size() > variable.id);
    return variables[variable.id];
}

u32& IRInterpreter::get_or_allocate(IRVariable& variable) {
    if (variables.size() <= variable.id) {
        variables.resize(variable.id + 1);
    }

    return variables[variable.id];
}

void IRInterpreter::assign_variable(IRVariable& variable, u32 value) {
    auto& allocated_variable = get_or_allocate(variable);
    allocated_variable = value;
}

u32 IRInterpreter::resolve_value(IRValue& value) {
    if (value.is_variable()) {
        return get(value.as_variable());
    } else {
        return value.as_constant().value;
    }
}

void IRInterpreter::handle_set_carry(IROpcodeVariant& opcode_variant) {
    logger.todo("IRInterpreter: handle_set_carry");
}

void IRInterpreter::handle_clear_carry(IROpcodeVariant& opcode_variant) {
    logger.todo("IRInterpreter: handle_clear_carry");
}

void IRInterpreter::handle_move(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRMove>(opcode_variant);
    auto value = resolve_value(opcode.src);
    assign_variable(opcode.dst, value);

    if (opcode.set_flags) {
        logger.todo("handle_move: handle set flags");
    }
}

void IRInterpreter::handle_load_gpr(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRLoadGPR>(opcode_variant);
    auto value = jit.get_gpr(opcode.src.gpr, opcode.src.mode);
    assign_variable(opcode.dst, value);
}

void IRInterpreter::handle_store_gpr(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRStoreGPR>(opcode_variant);
    auto value = resolve_value(opcode.src);
    jit.set_gpr(opcode.dst.gpr, opcode.dst.mode, value);
}

void IRInterpreter::handle_add(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRAdd>(opcode_variant);
    auto lhs = resolve_value(opcode.lhs);
    auto rhs = resolve_value(opcode.rhs);
    assign_variable(opcode.dst, lhs + rhs);

    if (opcode.set_flags) {
        logger.todo("handle_add: handle set flags");
    }
}

void IRInterpreter::handle_logical_shift_left(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRLogicalShiftLeft>(opcode_variant);

    logger.todo("IRInterpreter: handle_logical_shift_left");

    if (opcode.set_carry) {
        logger.todo("handle_logical_shift_left: handle set carry");
    }
}

void IRInterpreter::handle_and(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRAnd>(opcode_variant);

    logger.todo("IRInterpreter: handle_and");

    if (opcode.set_flags) {
        logger.todo("handle_and: handle set flags");
    }
}

void IRInterpreter::handle_logical_shift_right(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRLogicalShiftRight>(opcode_variant);

    logger.todo("IRInterpreter: handle_logical_shift_right");

    if (opcode.set_carry) {
        logger.todo("handle_logical_shift_right: handle set carry");
    }
}

} // namespace arm