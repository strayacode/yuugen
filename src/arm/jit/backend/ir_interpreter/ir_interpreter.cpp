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
    logger.debug("run at %08x", location.get_address());
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
    auto cpsr = jit.get_cpsr(); 
    bool n = cpsr.n;
    bool z = cpsr.z;
    bool c = cpsr.c;
    bool v = cpsr.v;

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
    case IROpcodeType::LoadGPR:
        return {&IRInterpreter::handle_load_gpr, *opcode->as<IRLoadGPR>()};
    case IROpcodeType::StoreGPR:
        return {&IRInterpreter::handle_store_gpr, *opcode->as<IRStoreGPR>()};
    case IROpcodeType::LoadCPSR:
        return {&IRInterpreter::handle_load_cpsr, *opcode->as<IRLoadCPSR>()};
    case IROpcodeType::StoreCPSR:
        return {&IRInterpreter::handle_store_cpsr, *opcode->as<IRStoreCPSR>()};
    case IROpcodeType::LoadSPSR:
        return {&IRInterpreter::handle_load_spsr, *opcode->as<IRLoadSPSR>()};
    case IROpcodeType::StoreSPSR:
        return {&IRInterpreter::handle_store_spsr, *opcode->as<IRStoreSPSR>()};
    case IROpcodeType::BitwiseAnd:
        return {&IRInterpreter::handle_bitwise_and, *opcode->as<IRBitwiseAnd>()};
    case IROpcodeType::BitwiseOr:
        return {&IRInterpreter::handle_bitwise_or, *opcode->as<IRBitwiseOr>()};
    case IROpcodeType::BitwiseNot:
        return {&IRInterpreter::handle_bitwise_not, *opcode->as<IRBitwiseNot>()};
    case IROpcodeType::Add:
        return {&IRInterpreter::handle_add, *opcode->as<IRAdd>()};
    case IROpcodeType::Sub:
        return {&IRInterpreter::handle_sub, *opcode->as<IRSub>()};
    case IROpcodeType::GetNZ:
        return {&IRInterpreter::handle_get_nz, *opcode->as<IRGetNZ>()};
    case IROpcodeType::GetNZCV:
        return {&IRInterpreter::handle_get_nzcv, *opcode->as<IRGetNZCV>()};
    case IROpcodeType::GetC:
        return {&IRInterpreter::handle_get_c, *opcode->as<IRGetC>()};
    case IROpcodeType::Branch:
        return {&IRInterpreter::handle_branch, *opcode->as<IRBranch>()};
    case IROpcodeType::BranchExchange:
        return {&IRInterpreter::handle_branch_exchange, *opcode->as<IRBranchExchange>()};
    case IROpcodeType::Copy:
        return {&IRInterpreter::handle_copy, *opcode->as<IRCopy>()};
    case IROpcodeType::LogicalShiftLeft:
        return {&IRInterpreter::handle_logical_shift_left, *opcode->as<IRLogicalShiftLeft>()};
    case IROpcodeType::LogicalShiftRight:
        return {&IRInterpreter::handle_logical_shift_right, *opcode->as<IRLogicalShiftRight>()};
    case IROpcodeType::MemoryWrite:
        return {&IRInterpreter::handle_memory_write, *opcode->as<IRMemoryWrite>()};
    case IROpcodeType::UpdateFlag:
        return {&IRInterpreter::handle_update_flag, *opcode->as<IRUpdateFlag>()};
    case IROpcodeType::StoreFlags:
        return {&IRInterpreter::handle_store_flags, *opcode->as<IRStoreFlags>()};
    case IROpcodeType::Compare:
        return {&IRInterpreter::handle_compare, *opcode->as<IRCompare>()};
    case IROpcodeType::ArithmeticShiftRight:
        return {&IRInterpreter::handle_arithmetic_shift_right, *opcode->as<IRArithmeticShiftRight>()};
    case IROpcodeType::RotateRight:
        return {&IRInterpreter::handle_rotate_right, *opcode->as<IRRotateRight>()};
    case IROpcodeType::MemoryRead:
        return {&IRInterpreter::handle_memory_read, *opcode->as<IRMemoryRead>()};
    case IROpcodeType::Multiply:
        return {&IRInterpreter::handle_multiply, *opcode->as<IRMultiply>()};
    case IROpcodeType::ExclusiveOr:
        return {&IRInterpreter::handle_exclusive_or, *opcode->as<IRExclusiveOr>()};
    case IROpcodeType::Test:
        return {&IRInterpreter::handle_test, *opcode->as<IRTest>()};
    case IROpcodeType::AddCarry:
        return {&IRInterpreter::handle_add_carry, *opcode->as<IRAddCarry>()};
    case IROpcodeType::MoveNegate:
        return {&IRInterpreter::handle_move_negate, *opcode->as<IRMoveNegate>()};
    case IROpcodeType::CompareNegate:
        return {&IRInterpreter::handle_compare_negate, *opcode->as<IRCompareNegate>()};
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

void IRInterpreter::handle_load_cpsr(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRLoadCPSR>(opcode_variant);
    logger.debug("cpsr load %08x", jit.state.cpsr.data);
    assign_variable(opcode.dst, jit.state.cpsr.data);
}

void IRInterpreter::handle_store_cpsr(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRStoreCPSR>(opcode_variant);
    auto src = resolve_value(opcode.src);
    StatusRegister psr;
    psr.data = src;
    jit.set_cpsr(psr);
}

void IRInterpreter::handle_load_spsr(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRLoadSPSR>(opcode_variant);
    logger.todo("IRInterpreter: handle_load_spsr");
}

void IRInterpreter::handle_store_spsr(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRStoreSPSR>(opcode_variant);
    logger.todo("IRInterpreter: handle_store_spsr");
}

void IRInterpreter::handle_bitwise_and(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRBitwiseAnd>(opcode_variant);
    auto lhs = resolve_value(opcode.lhs);
    auto rhs = resolve_value(opcode.rhs);
    assign_variable(opcode.dst, lhs & rhs);
}

void IRInterpreter::handle_bitwise_or(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRBitwiseOr>(opcode_variant);
    auto lhs = resolve_value(opcode.lhs);
    auto rhs = resolve_value(opcode.rhs);
    assign_variable(opcode.dst, lhs | rhs);
}

void IRInterpreter::handle_bitwise_not(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRBitwiseNot>(opcode_variant);
    auto src = resolve_value(opcode.src);
    assign_variable(opcode.dst, ~src);
}

void IRInterpreter::handle_add(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRAdd>(opcode_variant);
    auto lhs = resolve_value(opcode.lhs);
    auto rhs = resolve_value(opcode.rhs);
    auto result = lhs + rhs;
    
    assign_variable(opcode.dst, result);
    update_flag(Flags::C, result < lhs);
    update_flag(Flags::V, calculate_add_overflow(lhs, rhs, result));
}

void IRInterpreter::handle_sub(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRSub>(opcode_variant);
    auto lhs = resolve_value(opcode.lhs);
    auto rhs = resolve_value(opcode.rhs);
    auto result = lhs - rhs;
    
    assign_variable(opcode.dst, result);
    update_flag(Flags::C, lhs >= rhs);
    update_flag(Flags::V, calculate_sub_overflow(lhs, rhs, result));
}

void IRInterpreter::handle_get_nz(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRGetNZ>(opcode_variant);
    auto cpsr = resolve_value(opcode.cpsr);
    auto value = resolve_value(opcode.value);
    
    
    common::set_bit<30>(cpsr, value == 0);
    common::set_bit<31>(cpsr, value >> 31);
    assign_variable(opcode.dst, cpsr);
}

void IRInterpreter::handle_get_nzcv(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRGetNZCV>(opcode_variant);
    auto cpsr = resolve_value(opcode.cpsr);
    auto value = resolve_value(opcode.value);
    
    common::set_bit<28>(cpsr, flags & Flags::V);
    common::set_bit<29>(cpsr, flags & Flags::C);
    common::set_bit<30>(cpsr, value == 0);
    common::set_bit<31>(cpsr, value >> 31);
    assign_variable(opcode.dst, cpsr);
}

void IRInterpreter::handle_get_c(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRGetC>(opcode_variant);
    auto cpsr = resolve_value(opcode.cpsr);
    
    common::set_bit<29>(cpsr, flags & Flags::C);
    assign_variable(opcode.dst, cpsr);
}

void IRInterpreter::handle_branch(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRBranch>(opcode_variant);
    auto address = resolve_value(opcode.address);
    auto instruction_size = opcode.is_arm ? sizeof(u32) : sizeof(u16);
    auto address_mask = ~(instruction_size - 1);

    address += 2 * instruction_size;
    address &= address_mask;
    jit.set_gpr(GPR::PC, address);
}

void IRInterpreter::handle_branch_exchange(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRBranchExchange>(opcode_variant);
    auto address = resolve_value(opcode.address);
    bool is_arm;

    switch (opcode.exchange_type) {
    case ExchangeType::Bit0:
        is_arm = !(address & 0x1);
        break;
    }

    jit.state.cpsr.t = !is_arm;

    auto instruction_size = is_arm ? sizeof(u32) : sizeof(u16);
    auto address_mask = ~(instruction_size - 1);

    address &= address_mask;
    jit.set_gpr(GPR::PC, address);
}

void IRInterpreter::handle_copy(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRCopy>(opcode_variant);
    auto value = resolve_value(opcode.src);
    assign_variable(opcode.dst, value);
}

void IRInterpreter::handle_logical_shift_left(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRLogicalShiftLeft>(opcode_variant);
    auto value = resolve_value(opcode.src);
    auto amount = resolve_value(opcode.amount);
    auto [result, carry] = lsl(value, amount);
    assign_variable(opcode.dst, result);

    logger.debug("r0 %08x", jit.get_gpr(GPR::R0));

    logger.debug("lsl %08x %08x", value, amount);

    if (carry) {
        logger.debug("update carry to %d", *carry);
        update_flag(Flags::C, *carry);
    }
}

void IRInterpreter::handle_logical_shift_right(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRLogicalShiftRight>(opcode_variant);
    auto value = resolve_value(opcode.src);
    auto amount = resolve_value(opcode.amount);
    auto [result, carry] = lsr(value, amount, opcode.amount.is_constant());
    assign_variable(opcode.dst, result);

    if (carry) {
        update_flag(Flags::C, *carry);
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

void IRInterpreter::handle_update_flag(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRUpdateFlag>(opcode_variant);
    update_flag(opcode.flag, opcode.value);
}

void IRInterpreter::handle_store_flags(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRStoreFlags>(opcode_variant);
    u32 flags_mask = opcode.flags << 28;
    jit.state.cpsr.data = (jit.state.cpsr.data & ~flags_mask) | ((flags << 28) & flags_mask);
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

void IRInterpreter::handle_arithmetic_shift_right(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRArithmeticShiftRight>(opcode_variant);
    auto value = resolve_value(opcode.src);
    auto amount = resolve_value(opcode.amount);
    auto [result, carry] = asr(value, amount, opcode.amount.is_constant());
    assign_variable(opcode.dst, result);

    if (carry) {
        update_flag(Flags::C, *carry);
    }
}

void IRInterpreter::handle_rotate_right(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRRotateRight>(opcode_variant);
    auto value = resolve_value(opcode.src);
    auto amount = resolve_value(opcode.amount);
    auto [result, carry] = ror(value, amount);
    assign_variable(opcode.dst, result);

    if (carry) {
        update_flag(Flags::C, *carry);
    }
}

void IRInterpreter::handle_memory_read(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRMemoryRead>(opcode_variant);
    auto addr = resolve_value(opcode.addr);
    
    switch (opcode.access_size) {
    case AccessSize::Byte:
        if (opcode.access_type == AccessType::Signed) {
            logger.todo("handle signed byte read");
        } else {
            assign_variable(opcode.dst, jit.read_byte(addr));
        }
        
        break;
    case AccessSize::Half:
        switch (opcode.access_type) {
        case AccessType::Aligned:
            assign_variable(opcode.dst, jit.read_half(addr));
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
            assign_variable(opcode.dst, jit.read_word(addr));
        }

        break;
    }
}

void IRInterpreter::handle_multiply(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRMultiply>(opcode_variant);
    auto lhs = resolve_value(opcode.lhs);
    auto rhs = resolve_value(opcode.rhs);
    auto result = lhs * rhs;
    assign_variable(opcode.dst, result);

    if (opcode.set_flags) {
        update_flag(Flags::N, result >> 31);
        update_flag(Flags::Z, result == 0);
    }
}

void IRInterpreter::handle_exclusive_or(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRExclusiveOr>(opcode_variant);
    auto lhs = resolve_value(opcode.lhs);
    auto rhs = resolve_value(opcode.rhs);
    auto result = lhs ^ rhs;
    assign_variable(opcode.dst, result);

    if (opcode.set_flags) {
        update_flag(Flags::N, result >> 31);
        update_flag(Flags::Z, result == 0);
    }
}

void IRInterpreter::handle_test(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRTest>(opcode_variant);
    auto lhs = resolve_value(opcode.lhs);
    auto rhs = resolve_value(opcode.rhs);
    auto result = lhs & rhs;

    update_flag(Flags::N, result >> 31);
    update_flag(Flags::Z, result == 0);
}

void IRInterpreter::handle_add_carry(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRAddCarry>(opcode_variant);
    auto lhs = resolve_value(opcode.lhs);
    auto rhs = resolve_value(opcode.rhs);
    auto carry = flags & Flags::C;
    u64 result64 = static_cast<u64>(lhs) + static_cast<u64>(rhs) + static_cast<u64>(carry);
    u32 result = static_cast<u32>(result64);
    assign_variable(opcode.dst, result);

    if (opcode.set_flags) {
        update_flag(Flags::N, result >> 31);
        update_flag(Flags::Z, result == 0);
        update_flag(Flags::C, result64 >> 32);
        update_flag(Flags::V, calculate_add_overflow(lhs, rhs, result));
    }
}

void IRInterpreter::handle_move_negate(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRMoveNegate>(opcode_variant);
    auto value = resolve_value(opcode.src);
    auto result = ~value;
    assign_variable(opcode.dst, result);

    if (opcode.set_flags) {
        update_flag(Flags::N, result >> 31);
        update_flag(Flags::Z, result == 0);
    }
}

void IRInterpreter::handle_compare_negate(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRCompareNegate>(opcode_variant);
    auto lhs = resolve_value(opcode.lhs);
    auto rhs = resolve_value(opcode.rhs);
    auto result = lhs + rhs;

    update_flag(Flags::N, result >> 31);
    update_flag(Flags::Z, result == 0);
    update_flag(Flags::C, result < lhs);
    update_flag(Flags::V, calculate_add_overflow(lhs, rhs, result));
}

} // namespace arm