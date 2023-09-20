#pragma once

#include <memory>
#include "common/types.h"
#include "arm/jit/basic_block.h"
#include "arm/jit/ir/value.h"
#include "arm/cpu.h"

namespace arm {

class IREmitter {
public:
    IREmitter(BasicBlock& basic_block);

    IRVariable create_variable();

    // state opcodes
    IRVariable load_gpr(GPR gpr);
    void store_gpr(GPR gpr, IRValue src);
    IRVariable load_cpsr();
    void store_cpsr(IRVariable src);
    IRVariable load_spsr();
    void store_spsr(IRVariable src);
    void copy_spsr_to_cpsr();

    // bitwise opcodes
    IRVariable bitwise_and(IRValue lhs, IRValue rhs);

    // flag opcodes
    void update_nz(IRValue value);

    IRVariable move(IRValue src, bool set_flags);
    IRVariable add(IRValue lhs, IRValue rhs, bool set_flags);
    IRVariable logical_shift_left(IRValue src, IRValue amount, bool set_carry);
    
    IRVariable logical_shift_right(IRValue src, IRValue amount, bool set_carry);
    void memory_write(IRValue addr, IRVariable src, AccessSize access_size, AccessType access_type);
    IRVariable sub(IRValue lhs, IRValue rhs, bool set_flags);
    void update_flag(Flags flag, bool value);
    void store_flags(Flags flags);
    void compare(IRValue lhs, IRValue rhs);
    
    IRVariable or_(IRValue lhs, IRValue rhs, bool set_flags);
    IRVariable arithmetic_shift_right(IRValue src, IRValue amount, bool set_carry);
    IRVariable rotate_right(IRValue src, IRValue amount, bool set_carry);
    IRVariable memory_read(IRValue addr, AccessSize access_size, AccessType access_type);
    IRVariable bic(IRValue lhs, IRValue rhs, bool set_flags);
    void branch(IRValue address, bool is_arm);
    void branch_exchange(IRValue address, ExchangeType exchange_type);
    IRVariable multiply(IRValue lhs, IRValue rhs, bool set_flags);
    IRVariable exclusive_or(IRValue lhs, IRValue rhs, bool set_flags);
    void test(IRValue lhs, IRValue rhs);
    IRVariable add_carry(IRValue lhs, IRValue rhs, bool set_flags);
    IRVariable move_negate(IRValue src, bool set_flags);
    void compare_negate(IRValue lhs, IRValue rhs);

    BasicBlock& basic_block;

private:
    template <typename T, typename... Args>
    void push(Args... args) {
        basic_block.opcodes.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    u32 next_variable_id{0};
};

} // namespace arm