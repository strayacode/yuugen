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
    
    for (auto& opcode : basic_block.opcodes) {
        compiled_block.instructions.push_back(compile_ir_opcode(opcode));
    }

    code_cache.set(basic_block.location, std::move(compiled_block));
}

int IRInterpreter::run(Location location)  {
    auto& compiled_block = code_cache.get(location);
    for (auto& compiled_instruction : compiled_block.instructions) {
        (this->*compiled_instruction.fn)(compiled_instruction.opcode_variant);
        logger.debug("after executing instruction: pc: %08x", jit.get_gpr(GPR::PC));
    }

    return compiled_block.cycles;
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
    logger.debug("execute %s", opcode.to_string().c_str());
    auto value = resolve_value(opcode.src);
    assign_variable(opcode.dst, value);

    if (opcode.set_flags) {
        logger.todo("handle_move: handle set flags");
    }
}

void IRInterpreter::handle_load_gpr(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRLoadGPR>(opcode_variant);
    logger.debug("execute %s", opcode.to_string().c_str());
    auto value = jit.get_gpr(opcode.src.gpr, opcode.src.mode);
    assign_variable(opcode.dst, value);
}

void IRInterpreter::handle_store_gpr(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRStoreGPR>(opcode_variant);
    logger.debug("execute %s", opcode.to_string().c_str());
    auto& value = get(opcode.src);
    jit.set_gpr(opcode.dst.gpr, opcode.dst.mode, value);
}

void IRInterpreter::handle_add(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRAdd>(opcode_variant);
    logger.debug("execute %s", opcode.to_string().c_str());
    auto lhs = resolve_value(opcode.lhs);
    auto rhs = resolve_value(opcode.rhs);
    assign_variable(opcode.dst, lhs + rhs);

    if (opcode.set_flags) {
        logger.todo("handle_add: handle set flags");
    }
}

} // namespace arm