#include "common/logger.h"
#include "arm/jit/backend/ir_interpreter/ir_interpreter.h"

namespace arm {

void IRInterpreter::reset() {
    code_cache.reset();
}

bool IRInterpreter::has_code_at(Location location) {
    return code_cache.has_code_at(location);
}

void IRInterpreter::compile(BasicBlock& basic_block)  {
    std::vector<CompiledInstruction> compiled_instructions;

    for (auto& opcode : basic_block.opcodes) {
        compiled_instructions.push_back(compile_ir_opcode(opcode));
    }

    code_cache.set(basic_block.location, std::move(compiled_instructions));
}

void IRInterpreter::run(Location location)  {
    for (auto& compiled_instruction : code_cache.get(location)) {
        (this->*compiled_instruction.fn)(compiled_instruction.opcode_variant);
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

    

    logger.debug("dst %s src %s", opcode.dst.to_string().c_str(), opcode.src.to_string().c_str());
    logger.todo("IRInterpreter: handle_move");
}

void IRInterpreter::handle_load_gpr(IROpcodeVariant& opcode_variant) {
    logger.todo("IRInterpreter: handle_load_gpr");
}

void IRInterpreter::handle_store_gpr(IROpcodeVariant& opcode_variant) {
    logger.todo("IRInterpreter: handle_store_gpr");
}

void IRInterpreter::handle_add(IROpcodeVariant& opcode_variant) {
    logger.todo("IRInterpreter: handle_add");
}

} // namespace arm