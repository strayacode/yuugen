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
    IRVariable load_gpr(GPR gpr);
    IRVariable load_gpr(GPR gpr, Mode mode);
    void store_gpr(GPR gpr, IRValue src);
    void store_gpr(GPR gpr, Mode mode, IRValue src);
    IRVariable load_cpsr();
    void store_cpsr(IRVariable src);
    IRVariable load_spsr();
    void store_spsr(IRVariable src);
    void copy_spsr_to_cpsr();
    IRVariable load_coprocessor(u32 cn, u32 cm, u32 cp);
    void store_coprocessor(u32 cn, u32 cm, u32 cp, IRValue src);

    // bitwise opcodes
    IRVariable bitwise_and(IRValue lhs, IRValue rhs);
    IRVariable bitwise_or(IRValue lhs, IRValue rhs);
    IRVariable bitwise_not(IRValue src);
    IRVariable bitwise_exclusive_or(IRValue lhs, IRValue rhs);

    // arithmetic opcodes
    IRVariable add(IRValue lhs, IRValue rhs);
    IRPair<IRVariable> add_long(IRPair<IRValue> lhs, IRPair<IRValue> rhs);
    IRVariable subtract(IRValue lhs, IRValue rhs);
    IRVariable multiply(IRValue lhs, IRValue rhs);
    IRPair<IRVariable> multiply_long(IRValue lhs, IRValue rhs, bool is_signed);
    IRVariable logical_shift_left(IRValue src, IRValue amount);
    IRVariable logical_shift_right(IRValue src, IRValue amount);
    IRVariable arithmetic_shift_right(IRValue src, IRValue amount);
    IRPair<IRVariable> barrel_shifter_logical_shift_left(IRValue src, IRValue amount);
    IRPair<IRVariable> barrel_shifter_logical_shift_right(IRValue src, IRValue amount);
    IRPair<IRVariable> barrel_shifter_arithmetic_shift_right(IRValue src, IRValue amount);
    IRPair<IRVariable> barrel_shifter_rotate_right(IRValue src, IRValue amount);
    IRPair<IRVariable> barrel_shifter_rotate_right_extended(IRValue src, IRConstant amount);
    IRVariable sign_extend_byte(IRValue src);
    IRVariable sign_extend_half(IRValue src);
    IRVariable count_leading_zeroes(IRValue src);

    // flag opcodes
    IRVariable load_flag(Flag flag);
    void store_flag(Flag flag, IRValue value);
    void store_nz(IRValue value);
    void store_nz_long(IRPair<IRVariable> value);
    void store_add_cv(IRValue lhs, IRValue rhs, IRValue result);
    void store_sub_cv(IRValue lhs, IRValue rhs, IRValue result);
    void store_adc_cv(IRValue lhs, IRValue rhs, IRValue result);
    void store_sbc_cv(IRValue lhs, IRValue rhs, IRValue result);
    IRVariable add_overflow(IRValue lhs, IRValue rhs, IRValue result);
    IRVariable sub_overflow(IRValue lhs, IRValue rhs, IRValue result);
    IRVariable compare(IRValue lhs, IRValue rhs, CompareType compare_type);

    // branch opcodes
    void branch(IRValue address);
    void branch_exchange(IRValue address, ExchangeType exchange_type);

    // misc opcodes
    IRVariable copy(IRValue src);

    // helpers
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