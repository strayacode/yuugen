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
    TypedValue<Type::U32> load_gpr(GPR gpr, Mode mode);
    void store_gpr(GPR gpr, TypedValue<Type::U32> src);
    void store_gpr(GPR gpr, Mode mode, TypedValue<Type::U32> src);
    TypedValue<Type::U32> load_cpsr();
    void store_cpsr(TypedValue<Type::U32> src);
    TypedValue<Type::U32> load_spsr();
    void store_spsr(TypedValue<Type::U32> src);
    void store_spsr(TypedValue<Type::U32> src, Mode mode);
    void copy_spsr_to_cpsr();
    TypedValue<Type::U32> load_coprocessor(u32 cn, u32 cm, u32 cp);
    void store_coprocessor(u32 cn, u32 cm, u32 cp, IRValue src);

    // bitwise opcodes
    TypedValue<Type::U32> bitwise_and(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs);
    TypedValue<Type::U32> bitwise_or(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs);
    TypedValue<Type::U32> bitwise_not(TypedValue<Type::U32> src);
    TypedValue<Type::U32> bitwise_exclusive_or(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs);

    // arithmetic opcodes
    TypedValue<Type::U32> add(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs);
    IRPair<IRVariable> add_long(IRPair<IRValue> lhs, IRPair<IRValue> rhs);
    TypedValue<Type::U32> subtract(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs);
    TypedValue<Type::U32> subtract_with_carry(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs, TypedValue<Type::U1> carry);
    TypedValue<Type::U32> multiply(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs);
    IRPair<IRVariable> multiply_long(IRValue lhs, IRValue rhs, bool is_signed);
    TypedValue<Type::U32> logical_shift_left(TypedValue<Type::U32> src, TypedValue<Type::U8> amount);
    TypedValue<Type::U32> logical_shift_right(TypedValue<Type::U32> src, TypedValue<Type::U8> amount);
    TypedValue<Type::U32> arithmetic_shift_right(TypedValue<Type::U32> src, TypedValue<Type::U8> amount);
    TypedValue<Type::U32> rotate_right(TypedValue<Type::U32> src, TypedValue<Type::U8> amount);
    IRPair<IRVariable> barrel_shifter_logical_shift_left(TypedValue<Type::U32> src, TypedValue<Type::U8> amount);
    IRPair<IRVariable> barrel_shifter_logical_shift_right(TypedValue<Type::U32> src, TypedValue<Type::U8> amount);
    IRPair<IRVariable> barrel_shifter_arithmetic_shift_right(TypedValue<Type::U32> src, TypedValue<Type::U8> amount);
    IRPair<IRVariable> barrel_shifter_rotate_right(TypedValue<Type::U32> src, TypedValue<Type::U8> amount);
    IRPair<IRVariable> barrel_shifter_rotate_right_extended(TypedValue<Type::U32> src, TypedValue<Type::U8> amount);
    IRVariable sign_extend_byte(IRValue src);
    IRVariable sign_extend_half(IRValue src);
    IRVariable count_leading_zeroes(IRValue src);

    // flag opcodes
    TypedValue<Type::U1> load_flag(Flag flag);
    void store_flag(Flag flag, TypedValue<Type::U1> value);
    void store_nz(TypedValue<Type::U32> value);
    void store_nz_long(IRPair<TypedValue<Type::U32>> value);
    void store_add_cv(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs, TypedValue<Type::U32> result);
    void store_sub_cv(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs, TypedValue<Type::U32> result);
    void store_adc_cv(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs, TypedValue<Type::U32> result);
    void store_sbc_cv(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs, TypedValue<Type::U32> result);
    TypedValue<Type::U1> add_overflow(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs, TypedValue<Type::U32> result);
    TypedValue<Type::U1> sub_overflow(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs, TypedValue<Type::U32> result);
    TypedValue<Type::U1> compare(TypedValue<Type::U32> lhs, TypedValue<Type::U32> rhs, CompareType compare_type);

    // branch opcodes
    void branch(TypedValue<Type::U32> address);
    void branch_exchange(TypedValue<Type::U32> address, ExchangeType exchange_type);

    // misc opcodes
    TypedValue<Type::U32> copy(TypedValue<Type::U32> src);
    TypedValue<Type::U1> get_bit(TypedValue<Type::U32> src, TypedValue<Type::U8> bit);
    TypedValue<Type::U1> set_bit(TypedValue<Type::U32> src, TypedValue<Type::U1> value, TypedValue<Type::U8> bit);

    // helpers
    template <Type T>
    TypedValue<Type::U32> extend32(TypedValue<T> value) {
        static_assert(TypedValue<Type::U32>::is_larger<Type::U32, T>());
        return TypedValue<Type::U32>{value};
    }

    template <Type T>
    TypedValue<Type::U1> truncate1(TypedValue<T> value) {
        static_assert(TypedValue<Type::U1>::is_smaller<Type::U1, T>());
        return TypedValue<Type::U1>{value};
    }

    template <Type T>
    TypedValue<Type::U8> truncate_byte(TypedValue<T> value) {
        static_assert(TypedValue<Type::U8>::is_smaller<Type::U8, T>());
        return TypedValue<Type::U8>{value};
    }

    TypedValue<Type::U1> imm1(bool value);
    TypedValue<Type::U8> imm8(u8 value);
    TypedValue<Type::U32> imm32(u32 value);
    IRPair<IRValue> pair(IRValue first, IRValue second);
    IRPair<IRVariable> barrel_shifter(IRValue value, ShiftType shift_type, IRValue amount);
    void link();
    void advance_pc();
    void flush_pipeline();

    void memory_write_byte(TypedValue<Type::U32> addr, TypedValue<Type::U8> src);
    void memory_write_half(TypedValue<Type::U32> addr, TypedValue<Type::U16> src);
    void memory_write_word(TypedValue<Type::U32> addr, TypedValue<Type::U32> src);

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