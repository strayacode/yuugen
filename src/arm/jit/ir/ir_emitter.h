#pragma once

#include <memory>
#include "common/types.h"
#include "arm/jit/basic_block.h"
#include "arm/jit/ir/value.h"
#include "arm/cpu.h"
#include "arm/instructions.h"

namespace arm {

class IREmitter {
public:
    IREmitter(BasicBlock& basic_block);

    IRVariable create_variable();
    IRPair<IRVariable> create_pair();

    // state opcodes
    TypedValue<Type::U32> load_gpr(GPR gpr);
    IRVariable load_gpr(GPR gpr, Mode mode);
    void store_gpr(GPR gpr, IRValue src);
    void store_gpr(GPR gpr, Mode mode, IRValue src);
    IRVariable load_cpsr();
    void store_cpsr(IRVariable src);
    IRVariable load_spsr();
    void store_spsr(IRVariable src);
    void store_spsr(IRVariable src, Mode mode);
    void copy_spsr_to_cpsr();
    IRVariable load_coprocessor(u32 cn, u32 cm, u32 cp);
    void store_coprocessor(u32 cn, u32 cm, u32 cp, IRValue src);

    // bitwise opcodes
    IRVariable bitwise_and(IRValue lhs, IRValue rhs);
    IRVariable bitwise_or(IRValue lhs, IRValue rhs);
    IRVariable bitwise_not(IRValue src);
    TypedValue<Type::U32> bitwise_exclusive_or(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs);

    // arithmetic opcodes
    IRVariable add(IRValue lhs, IRValue rhs);
    IRPair<IRVariable> add_long(IRPair<IRValue> lhs, IRPair<IRValue> rhs);
    IRVariable subtract(IRValue lhs, IRValue rhs);
    TypedValue<Type::U32> subtract_with_carry(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs, TypedValue<Type::U1> carry);
    IRVariable multiply(IRValue lhs, IRValue rhs);
    IRPair<IRVariable> multiply_long(IRValue lhs, IRValue rhs, bool is_signed);
    IRVariable logical_shift_left(IRValue src, IRValue amount);
    IRVariable logical_shift_right(IRValue src, IRValue amount);
    IRVariable arithmetic_shift_right(IRValue src, IRValue amount);
    IRVariable rotate_right(IRValue src, IRValue amount);
    IRPair<IRVariable> barrel_shifter_logical_shift_left(IRValue src, IRValue amount);
    IRPair<IRVariable> barrel_shifter_logical_shift_right(IRValue src, IRValue amount);
    IRPair<IRVariable> barrel_shifter_arithmetic_shift_right(IRValue src, IRValue amount);
    IRPair<IRVariable> barrel_shifter_rotate_right(IRValue src, IRValue amount);
    IRPair<IRVariable> barrel_shifter_rotate_right_extended(IRValue src, IRConstant amount);
    IRVariable sign_extend_byte(IRValue src);
    IRVariable sign_extend_half(IRValue src);
    IRVariable count_leading_zeroes(IRValue src);

    // flag opcodes
    TypedValue<Type::U1> load_flag(Flag flag);
    void store_flag(Flag flag, IRValue value);
    void store_nz(IRValue value);
    void store_nz_long(IRPair<IRVariable> value);
    void store_add_cv(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs, TypedValue<Type::U32> result);
    void store_sub_cv(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs, TypedValue<Type::U32> result);
    void store_adc_cv(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs, TypedValue<Type::U32> result);
    void store_sbc_cv(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs, TypedValue<Type::U32> result);
    IRVariable add_overflow(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs, TypedValue<Type::U32> result);
    IRVariable sub_overflow(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs, TypedValue<Type::U32> result);
    IRVariable compare(IRValue lhs, IRValue rhs, CompareType compare_type);

    // branch opcodes
    void branch(IRValue address);
    void branch_exchange(IRValue address, ExchangeType exchange_type);

    // misc opcodes
    TypedValue<Type::U32> copy(TypedValue<Type::U32> src);
    IRPair<IRVariable> pair_copy(IRPair<IRValue> pair);
    IRVariable get_bit(IRValue src, IRValue bit);
    IRVariable set_bit(IRValue src, IRValue value, IRValue bit);

    // helpers
    template <Type T>
    TypedValue<Type::U32> extend32(TypedValue<T> value) {
        static_assert(TypedValue<Type::U32>::is_larger<Type::U32, T>());
        return TypedValue<Type::U32>{value};
    }

    TypedValue<Type::U1> imm1(bool value);
    TypedValue<Type::U32> imm32(u32 value);
    IRConstant constant(u32 value);
    IRPair<IRValue> pair(IRValue first, IRValue second);
    IRPair<IRVariable> barrel_shifter(IRValue value, ShiftType shift_type, IRValue amount);
    void link();
    void advance_pc();
    void flush_pipeline();

    void memory_write(IRValue addr, IRVariable src, AccessSize access_size);
    
    IRVariable memory_read(IRValue addr, AccessSize access_size, AccessType access_type);
    
    BasicBlock& basic_block;

private:
    template <typename T, typename... Args>
    void push(Args... args) {
        basic_block.opcodes.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    u32 next_variable_id{0};
};

} // namespace arm