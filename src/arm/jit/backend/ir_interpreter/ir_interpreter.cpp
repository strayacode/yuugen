#include <cassert>
#include "common/logger.h"
#include "arm/arithmetic.h"
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
    bool n = flags & Flags::N;
    bool z = flags & Flags::Z;
    bool c = flags & Flags::C;
    bool v = flags & Flags::V;

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
    case IROpcodeType::MemoryWrite:
        return {&IRInterpreter::handle_memory_write, *opcode->as<IRMemoryWrite>()};
    case IROpcodeType::Sub:
        return {&IRInterpreter::handle_sub, *opcode->as<IRSub>()};
    case IROpcodeType::UpdateFlag:
        return {&IRInterpreter::handle_update_flag, *opcode->as<IRUpdateFlag>()};
    case IROpcodeType::StoreFlags:
        return {&IRInterpreter::handle_store_flags, *opcode->as<IRStoreFlags>()};
    case IROpcodeType::Compare:
        return {&IRInterpreter::handle_compare, *opcode->as<IRCompare>()};
    case IROpcodeType::LoadCPSR:
        return {&IRInterpreter::handle_load_cpsr, *opcode->as<IRLoadCPSR>()};
    case IROpcodeType::LoadSPSR:
        return {&IRInterpreter::handle_load_spsr, *opcode->as<IRLoadSPSR>()};
    case IROpcodeType::Or:
        return {&IRInterpreter::handle_or, *opcode->as<IROr>()};
    case IROpcodeType::StoreCPSR:
        return {&IRInterpreter::handle_store_cpsr, *opcode->as<IRStoreCPSR>()};
    case IROpcodeType::StoreSPSR:
        return {&IRInterpreter::handle_store_spsr, *opcode->as<IRStoreSPSR>()};
    case IROpcodeType::ArithmeticShiftRight:
        return {&IRInterpreter::handle_arithmetic_shift_right, *opcode->as<IRArithmeticShiftRight>()};
    case IROpcodeType::RotateRight:
        return {&IRInterpreter::handle_rotate_right, *opcode->as<IRRotateRight>()};
    case IROpcodeType::MemoryRead:
        return {&IRInterpreter::handle_memory_read, *opcode->as<IRMemoryRead>()};
    case IROpcodeType::Bic:
        return {&IRInterpreter::handle_bic, *opcode->as<IRBic>()};
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

void IRInterpreter::update_flag(Flags to_update, bool value) {
    if (value) {
        flags = static_cast<Flags>(flags | to_update);
    } else {
        flags = static_cast<Flags>(flags & ~to_update);
    }
}

void IRInterpreter::dump_variables() {
    for (u64 i = 0; i < variables.size(); i++) {
        logger.debug("%%%d: %08x", i, variables[i]);
    }
}

void IRInterpreter::handle_move(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRMove>(opcode_variant);
    auto value = resolve_value(opcode.src);
    assign_variable(opcode.dst, value);

    if (opcode.set_flags) {
        update_flag(Flags::N, value >> 31);
        update_flag(Flags::Z, value == 0);
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
    auto result = lhs + rhs;
    assign_variable(opcode.dst, result);

    if (opcode.set_flags) {
        logger.todo("handle_add: handle set flags");
    }
}

void IRInterpreter::handle_logical_shift_left(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRLogicalShiftLeft>(opcode_variant);
    auto value = resolve_value(opcode.src);
    auto amount = resolve_value(opcode.amount);
    auto [result, carry] = lsl(value, amount);
    assign_variable(opcode.dst, result);

    if (opcode.set_carry && carry) {
        update_flag(Flags::C, *carry);
    }
}

void IRInterpreter::handle_and(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRAnd>(opcode_variant);
    auto lhs = resolve_value(opcode.lhs);
    auto rhs = resolve_value(opcode.rhs);
    auto result = lhs & rhs;
    assign_variable(opcode.dst, result);

    if (opcode.set_flags) {
        update_flag(Flags::N, result >> 31);
        update_flag(Flags::Z, result == 0);
    }
}

void IRInterpreter::handle_logical_shift_right(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRLogicalShiftRight>(opcode_variant);

    logger.todo("IRInterpreter: handle_logical_shift_right");

    if (opcode.set_carry) {
        logger.todo("handle_logical_shift_right: handle set carry");
    }
}

void IRInterpreter::handle_memory_write(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRMemoryWrite>(opcode_variant);
    auto addr = resolve_value(opcode.addr);
    auto src = resolve_value(opcode.src);

    switch (opcode.access_size) {
    case AccessSize::Byte:
        jit.write_byte(addr, src);
        break;
    case AccessSize::Half:
        jit.write_half(addr, src);
        break;
    case AccessSize::Word:
        jit.write_word(addr, src);
        break;
    }
}

void IRInterpreter::handle_sub(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRSub>(opcode_variant);
    auto lhs = resolve_value(opcode.lhs);
    auto rhs = resolve_value(opcode.rhs);
    auto result = lhs - rhs;
    
    assign_variable(opcode.dst, result);

    if (opcode.set_flags) {
        update_flag(Flags::N, result >> 31);
        update_flag(Flags::Z, result == 0);
        update_flag(Flags::C, lhs >= rhs);
        update_flag(Flags::V, calculate_sub_overflow(lhs, rhs, result));
    }
}

void IRInterpreter::handle_update_flag(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRUpdateFlag>(opcode_variant);
    update_flag(opcode.flag, opcode.value);
}

void IRInterpreter::handle_store_flags(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRStoreFlags>(opcode_variant);
    u32 flags_mask = opcode.flags << 28;
    jit.state.cpsr.data = (jit.state.cpsr.data & ~flags_mask) | (flags & flags_mask);
}

void IRInterpreter::handle_compare(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRCompare>(opcode_variant);
    auto lhs = resolve_value(opcode.lhs);
    auto rhs = resolve_value(opcode.rhs);
    auto result = lhs - rhs;

    update_flag(Flags::N, result >> 31);
    update_flag(Flags::Z, result == 0);
    update_flag(Flags::C, lhs >= rhs);
    update_flag(Flags::V, calculate_sub_overflow(lhs, rhs, result));
}

void IRInterpreter::handle_load_cpsr(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRLoadCPSR>(opcode_variant);
    assign_variable(opcode.dst, jit.state.cpsr.data);
}

void IRInterpreter::handle_load_spsr(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRLoadSPSR>(opcode_variant);
    logger.todo("IRInterpreter: handle_load_spsr");
}

void IRInterpreter::handle_or(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IROr>(opcode_variant);
    auto lhs = resolve_value(opcode.lhs);
    auto rhs = resolve_value(opcode.rhs);
    auto result = lhs | rhs;
    assign_variable(opcode.dst, result);

    if (opcode.set_flags) {
        update_flag(Flags::N, result >> 31);
        update_flag(Flags::Z, result == 0);
    }
}

void IRInterpreter::handle_store_cpsr(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRStoreCPSR>(opcode_variant);
    auto src = resolve_value(opcode.src);
    StatusRegister psr;
    psr.data = src;
    jit.set_cpsr(psr);
}

void IRInterpreter::handle_store_spsr(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRStoreSPSR>(opcode_variant);
    logger.todo("IRInterpreter: handle_store_spsr");
}

void IRInterpreter::handle_arithmetic_shift_right(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRArithmeticShiftRight>(opcode_variant);
    auto value = resolve_value(opcode.src);
    auto amount = resolve_value(opcode.amount);
    auto [result, carry] = asr(value, amount, opcode.amount.is_constant());
    assign_variable(opcode.dst, result);

    if (opcode.set_carry && carry) {
        update_flag(Flags::C, *carry);
    }
}

void IRInterpreter::handle_rotate_right(IROpcodeVariant& opcode_variant) {
    logger.todo("IRInterpreter: handle_rotate_right");
}

void IRInterpreter::handle_memory_read(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRMemoryRead>(opcode_variant);
    auto addr = resolve_value(opcode.addr);
    
    switch (opcode.access_size) {
    case AccessSize::Byte:
        if (opcode.access_type == AccessType::Signed) {
            logger.todo("handle signed byte read");
        } else {
            logger.todo("handle regular byte read");
        }
        
        break;
    case AccessSize::Half:
        switch (opcode.access_type) {
        case AccessType::Aligned:
            logger.todo("handle aligned half read");
            break;
        case AccessType::Unaligned:
            logger.todo("handle unaligned half read");
            break;
        case AccessType::Signed:
            logger.todo("handle signed half read");
            break;
        }
        
        break;
    case AccessSize::Word:
        if (opcode.access_type == AccessType::Unaligned) {
            assign_variable(opcode.dst, jit.read_word_rotate(addr));
        } else {
            logger.todo("handle regular word read");
        }

        break;
    }
}

void IRInterpreter::handle_bic(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRBic>(opcode_variant);
    auto lhs = resolve_value(opcode.lhs);
    auto rhs = resolve_value(opcode.rhs);
    auto result = lhs & ~rhs;
    assign_variable(opcode.dst, result);

    if (opcode.set_flags) {
        update_flag(Flags::N, result >> 31);
        update_flag(Flags::Z, result == 0);
    }
}

} // namespace arm