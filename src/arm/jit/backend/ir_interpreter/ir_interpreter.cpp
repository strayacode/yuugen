#include <cassert>
#include "common/logger.h"
#include "common/bits.h"
#include "arm/arithmetic.h"
#include "arm/jit/jit.h"
#include "arm/jit/backend/ir_interpreter/ir_interpreter.h"

namespace arm {

IRInterpreter::IRInterpreter(Jit& jit) : jit(jit) {}

void IRInterpreter::reset() {
    code_cache.reset();
    variables.clear();
}

Code IRInterpreter::get_code_at(Location location) {
    if (code_cache.has_code_at(location)) {
        return Code(location.value);
    } else {
        return nullptr;
    }
}

Code IRInterpreter::compile(BasicBlock& basic_block)  {
    CompiledBlock compiled_block;
    compiled_block.cycles = basic_block.cycles;
    compiled_block.num_instructions = basic_block.num_instructions;
    compiled_block.condition = basic_block.condition;
    compiled_block.location = basic_block.location;

    for (auto& opcode : basic_block.opcodes) {
        compiled_block.instructions.push_back(compile_ir_opcode(opcode));
    }

    code_cache.set(basic_block.location, std::move(compiled_block));
    return Code(basic_block.location.value);
}

int IRInterpreter::run(Code code, int cycles_left)  {
    auto location = Location{reinterpret_cast<u64>(code)};
    auto& compiled_block = code_cache.get_or_create(location);
    
    if (evaluate_condition(compiled_block.condition)) {
        for (auto& compiled_instruction : compiled_block.instructions) {
            (this->*compiled_instruction.fn)(compiled_instruction.opcode_variant);
        }

        return cycles_left - compiled_block.cycles;
    } else {
        u32 pc_after_block = jit.get_gpr(GPR::PC) + ((compiled_block.num_instructions - 2) * compiled_block.location.get_instruction_size());
        jit.set_gpr(GPR::PC, pc_after_block);

        // when all instructions fail the condition check,
        // then each one takes 1 cycle (for fetching the instruction)
        return cycles_left - compiled_block.num_instructions;
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
    case IROpcodeType::LoadCoprocessor:
        return {&IRInterpreter::handle_load_coprocessor, *opcode->as<IRLoadCoprocessor>()};
    case IROpcodeType::StoreCoprocessor:
        return {&IRInterpreter::handle_store_coprocessor, *opcode->as<IRStoreCoprocessor>()};
    case IROpcodeType::BitwiseAnd:
        return {&IRInterpreter::handle_bitwise_and, *opcode->as<IRBitwiseAnd>()};
    case IROpcodeType::BitwiseOr:
        return {&IRInterpreter::handle_bitwise_or, *opcode->as<IRBitwiseOr>()};
    case IROpcodeType::BitwiseNot:
        return {&IRInterpreter::handle_bitwise_not, *opcode->as<IRBitwiseNot>()};
    case IROpcodeType::BitwiseExclusiveOr:
        return {&IRInterpreter::handle_bitwise_exclusive_or, *opcode->as<IRBitwiseExclusiveOr>()};
    case IROpcodeType::Add:
        return {&IRInterpreter::handle_add, *opcode->as<IRAdd>()};
    case IROpcodeType::AddLong:
        return {&IRInterpreter::handle_add_long, *opcode->as<IRAddLong>()};
    case IROpcodeType::Subtract:
        return {&IRInterpreter::handle_subtract, *opcode->as<IRSubtract>()};
    case IROpcodeType::Multiply:
        return {&IRInterpreter::handle_multiply, *opcode->as<IRMultiply>()};
    case IROpcodeType::MultiplyLong:
        return {&IRInterpreter::handle_multiply_long, *opcode->as<IRMultiplyLong>()};
    case IROpcodeType::LogicalShiftLeft:
        return {&IRInterpreter::handle_logical_shift_left, *opcode->as<IRLogicalShiftLeft>()};
    case IROpcodeType::LogicalShiftRight:
        return {&IRInterpreter::handle_logical_shift_right, *opcode->as<IRLogicalShiftRight>()};
    case IROpcodeType::ArithmeticShiftRight:
        return {&IRInterpreter::handle_arithmetic_shift_right, *opcode->as<IRArithmeticShiftRight>()};
    case IROpcodeType::RotateRight:
        return {&IRInterpreter::handle_rotate_right, *opcode->as<IRRotateRight>()};
    case IROpcodeType::BarrelShifterLogicalShiftLeft:
        return {&IRInterpreter::handle_barrel_shifter_logical_shift_left, *opcode->as<IRBarrelShifterLogicalShiftLeft>()};
    case IROpcodeType::BarrelShifterLogicalShiftRight:
        return {&IRInterpreter::handle_barrel_shifter_logical_shift_right, *opcode->as<IRBarrelShifterLogicalShiftRight>()};
    case IROpcodeType::BarrelShifterArithmeticShiftRight:
        return {&IRInterpreter::handle_barrel_shifter_arithmetic_shift_right, *opcode->as<IRBarrelShifterArithmeticShiftRight>()};
    case IROpcodeType::BarrelShifterRotateRight:
        return {&IRInterpreter::handle_barrel_shifter_rotate_right, *opcode->as<IRBarrelShifterRotateRight>()};
    case IROpcodeType::BarrelShifterRotateRightExtended:
        return {&IRInterpreter::handle_barrel_shifter_rotate_right_extended, *opcode->as<IRBarrelShifterRotateRightExtended>()};
    case IROpcodeType::CountLeadingZeroes:
        return {&IRInterpreter::handle_count_leading_zeroes, *opcode->as<IRCountLeadingZeroes>()};
    case IROpcodeType::Compare:
        return {&IRInterpreter::handle_compare, *opcode->as<IRCompare>()};
    case IROpcodeType::Copy:
        return {&IRInterpreter::handle_copy, *opcode->as<IRCopy>()};
    case IROpcodeType::GetBit:
        return {&IRInterpreter::handle_get_bit, *opcode->as<IRGetBit>()};
    case IROpcodeType::SetBit:
        return {&IRInterpreter::handle_set_bit, *opcode->as<IRSetBit>()};
    case IROpcodeType::MemoryWrite:
        return {&IRInterpreter::handle_memory_write, *opcode->as<IRMemoryWrite>()};
    case IROpcodeType::MemoryRead:
        return {&IRInterpreter::handle_memory_read, *opcode->as<IRMemoryRead>()};
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

u64 IRInterpreter::resolve_pair(IRPair<IRValue>& pair) {
    return (static_cast<u64>(resolve_value(pair.first)) << 32) | static_cast<u64>(resolve_value(pair.second));
}

void IRInterpreter::dump_variables() {
    for (u64 i = 0; i < variables.size(); i++) {
        LOG_DEBUG("%%%d: %08x", i, variables[i]);
    }
}

void IRInterpreter::handle_load_gpr(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRLoadGPR>(opcode_variant);
    auto value = *jit.get_pointer_to_gpr(opcode.src.gpr, opcode.src.mode);
    assign_variable(opcode.dst, value);
}

void IRInterpreter::handle_store_gpr(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRStoreGPR>(opcode_variant);
    auto value = resolve_value(opcode.src);
    *jit.get_pointer_to_gpr(opcode.dst.gpr, opcode.dst.mode) = value;
}

void IRInterpreter::handle_load_cpsr(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRLoadCPSR>(opcode_variant);
    auto value = *jit.get_pointer_to_cpsr();
    assign_variable(opcode.dst, value.data);
}

void IRInterpreter::handle_store_cpsr(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRStoreCPSR>(opcode_variant);
    auto value = resolve_value(opcode.src);
    jit.get_pointer_to_cpsr()->data = value;
}

void IRInterpreter::handle_load_spsr(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRLoadSPSR>(opcode_variant);
    auto value = *jit.get_pointer_to_spsr(opcode.mode);
    assign_variable(opcode.dst, value.data);
}

void IRInterpreter::handle_store_spsr(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRStoreSPSR>(opcode_variant);
    auto value = resolve_value(opcode.src);
    jit.get_pointer_to_spsr(opcode.mode)->data = value;
}

void IRInterpreter::handle_load_coprocessor(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRLoadCoprocessor>(opcode_variant);
    auto value = jit.coprocessor.read(opcode.cn, opcode.cm, opcode.cp);
    assign_variable(opcode.dst, value);
}

void IRInterpreter::handle_store_coprocessor(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRStoreCoprocessor>(opcode_variant);
    auto value = resolve_value(opcode.src);
    jit.coprocessor.write(opcode.cn, opcode.cm, opcode.cp, value);
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

void IRInterpreter::handle_bitwise_exclusive_or(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRBitwiseExclusiveOr>(opcode_variant);
    auto lhs = resolve_value(opcode.lhs);
    auto rhs = resolve_value(opcode.rhs);
    assign_variable(opcode.dst, lhs ^ rhs);
}

void IRInterpreter::handle_add(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRAdd>(opcode_variant);
    auto lhs = resolve_value(opcode.lhs);
    auto rhs = resolve_value(opcode.rhs);
    assign_variable(opcode.dst, lhs + rhs);
}

void IRInterpreter::handle_add_long(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRAddLong>(opcode_variant);
    auto lhs = resolve_pair(opcode.lhs);
    auto rhs = resolve_pair(opcode.rhs);
    auto result = lhs + rhs;
    assign_variable(opcode.dst.first, result >> 32);
    assign_variable(opcode.dst.second, result);
}

void IRInterpreter::handle_subtract(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRSubtract>(opcode_variant);
    auto lhs = resolve_value(opcode.lhs);
    auto rhs = resolve_value(opcode.rhs);
    assign_variable(opcode.dst, lhs - rhs);
}

void IRInterpreter::handle_multiply(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRMultiply>(opcode_variant);
    auto lhs = resolve_value(opcode.lhs);
    auto rhs = resolve_value(opcode.rhs);
    assign_variable(opcode.dst, lhs * rhs);
}

void IRInterpreter::handle_multiply_long(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRMultiplyLong>(opcode_variant);

    if (opcode.is_signed) {
        s64 lhs = static_cast<s32>(resolve_value(opcode.lhs));
        s64 rhs = static_cast<s32>(resolve_value(opcode.rhs));
        s64 result = lhs * rhs;
        assign_variable(opcode.dst.first, result >> 32);
        assign_variable(opcode.dst.second, result);
    } else {
        u64 lhs = static_cast<u64>(resolve_value(opcode.lhs));
        u64 rhs = static_cast<u64>(resolve_value(opcode.rhs));
        u64 result = lhs * rhs;
        assign_variable(opcode.dst.first, result >> 32);
        assign_variable(opcode.dst.second, result);
    }
}

void IRInterpreter::handle_compare(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRCompare>(opcode_variant);
    auto lhs = resolve_value(opcode.lhs);
    auto rhs = resolve_value(opcode.rhs);

    switch (opcode.compare_type) {
    case CompareType::Equal:
        assign_variable(opcode.dst, lhs == rhs);
        break;
    case CompareType::LessThan:
        assign_variable(opcode.dst, lhs < rhs);
        break;
    case CompareType::GreaterEqual:
        assign_variable(opcode.dst, lhs >= rhs);
        break;
    case CompareType::GreaterThan:
        assign_variable(opcode.dst, lhs > rhs);
        break;
    }
}

void IRInterpreter::handle_copy(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRCopy>(opcode_variant);
    auto value = resolve_value(opcode.src);
    assign_variable(opcode.dst, value);
}

void IRInterpreter::handle_get_bit(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRGetBit>(opcode_variant);
    auto src = resolve_value(opcode.src);
    auto bit = resolve_value(opcode.bit);
    assign_variable(opcode.dst, (src >> bit) & 0x1);
}

void IRInterpreter::handle_set_bit(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRSetBit>(opcode_variant);
    auto src = resolve_value(opcode.src);
    auto value = resolve_value(opcode.value);
    auto bit = resolve_value(opcode.bit);
    assign_variable(opcode.dst, (src & ~(1 << bit)) | (value << bit));
}

void IRInterpreter::handle_logical_shift_left(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRLogicalShiftLeft>(opcode_variant);
    auto value = resolve_value(opcode.src);
    auto amount = resolve_value(opcode.amount);
    assign_variable(opcode.dst, value << amount);
}

void IRInterpreter::handle_logical_shift_right(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRLogicalShiftRight>(opcode_variant);
    auto value = resolve_value(opcode.src);
    auto amount = resolve_value(opcode.amount);
    assign_variable(opcode.dst, value >> amount);
}

void IRInterpreter::handle_arithmetic_shift_right(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRArithmeticShiftRight>(opcode_variant);
    auto value = resolve_value(opcode.src);
    auto amount = resolve_value(opcode.amount);
    assign_variable(opcode.dst, static_cast<s32>(value) >> amount);
}

void IRInterpreter::handle_rotate_right(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRRotateRight>(opcode_variant);
    auto value = resolve_value(opcode.src);
    auto amount = resolve_value(opcode.amount);
    assign_variable(opcode.dst, common::rotate_right(value, amount));
}

void IRInterpreter::handle_barrel_shifter_logical_shift_left(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRBarrelShifterLogicalShiftLeft>(opcode_variant);
    auto value = resolve_value(opcode.src);
    auto amount = resolve_value(opcode.amount);
    auto carry_in = resolve_value(opcode.carry);
    auto [result, carry] = lsl(value, amount);
    
    assign_variable(opcode.result_and_carry.first, result);

    if (carry) {
        assign_variable(opcode.result_and_carry.second, *carry);
    } else {
        assign_variable(opcode.result_and_carry.second, carry_in);
    }
}

void IRInterpreter::handle_barrel_shifter_logical_shift_right(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRBarrelShifterLogicalShiftRight>(opcode_variant);
    auto value = resolve_value(opcode.src);
    auto amount = resolve_value(opcode.amount);
    auto carry_in = resolve_value(opcode.carry);
    auto [result, carry] = lsr(value, amount, opcode.imm);
    
    assign_variable(opcode.result_and_carry.first, result);

    if (carry) {
        assign_variable(opcode.result_and_carry.second, *carry);
    } else {
        assign_variable(opcode.result_and_carry.second, carry_in);
    }
}

void IRInterpreter::handle_barrel_shifter_arithmetic_shift_right(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRBarrelShifterArithmeticShiftRight>(opcode_variant);
    auto value = resolve_value(opcode.src);
    auto amount = resolve_value(opcode.amount);
    auto carry_in = resolve_value(opcode.carry);
    auto [result, carry] = asr(value, amount, opcode.imm);
    
    assign_variable(opcode.result_and_carry.first, result);

    if (carry) {
        assign_variable(opcode.result_and_carry.second, *carry);
    } else {
        assign_variable(opcode.result_and_carry.second, carry_in);
    }
}

void IRInterpreter::handle_barrel_shifter_rotate_right(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRBarrelShifterRotateRight>(opcode_variant);
    auto value = resolve_value(opcode.src);
    auto amount = resolve_value(opcode.amount);
    auto carry_in = resolve_value(opcode.carry);
    auto [result, carry] = ror(value, amount);
    
    assign_variable(opcode.result_and_carry.first, result);

    if (carry) {
        assign_variable(opcode.result_and_carry.second, *carry);
    } else {
        assign_variable(opcode.result_and_carry.second, carry_in);
    }
}

void IRInterpreter::handle_barrel_shifter_rotate_right_extended(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRBarrelShifterRotateRightExtended>(opcode_variant);
    auto value = resolve_value(opcode.src);
    auto carry_in = resolve_value(opcode.carry);
    auto [result, carry] = rrx(value, carry_in);
    
    assign_variable(opcode.result_and_carry.first, result);

    if (carry) {
        assign_variable(opcode.result_and_carry.second, *carry);
    } else {
        assign_variable(opcode.result_and_carry.second, carry_in);
    }
}

void IRInterpreter::handle_count_leading_zeroes(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRCountLeadingZeroes>(opcode_variant);
    auto value = resolve_value(opcode.src);
    auto result = common::countl_zeroes(value);
    assign_variable(opcode.dst, result);
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

void IRInterpreter::handle_memory_read(IROpcodeVariant& opcode_variant) {
    auto& opcode = std::get<IRMemoryRead>(opcode_variant);
    auto addr = resolve_value(opcode.addr);
    
    switch (opcode.access_size) {
    case AccessSize::Byte:
        assign_variable(opcode.dst, jit.read_byte(addr));
        break;
    case AccessSize::Half:
        switch (opcode.access_type) {
        case AccessType::Aligned:
            assign_variable(opcode.dst, jit.read_half(addr));
            break;
        case AccessType::Unaligned:
            LOG_TODO("IRInterpreter: handle unaligned half read");
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

} // namespace arm