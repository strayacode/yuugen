#pragma once

#include <memory>
#include "common/types.h"
#include "arm/jit/basic_block.h"
#include "arm/jit/ir/types.h"
#include "arm/cpu.h"

namespace arm {

class Emitter {
public:
    Emitter(BasicBlock& basic_block);

    IRVariable create_variable();

    void set_carry();
    void clear_carry();
    IRVariable move(IRValue src, bool set_flags);
    IRVariable load_gpr(GPR gpr);
    void store_gpr(GPR gpr, IRValue src);
    IRVariable add(IRValue lhs, IRValue rhs, bool set_flags);
    IRVariable logical_shift_left(IRValue operand, IRValue amount, bool set_carry);
    IRVariable _and(IRValue lhs, IRValue rhs, bool set_flags);
    IRVariable logical_shift_right(IRValue operand, IRValue amount, bool set_carry);
    void memory_write(IRValue addr, IRVariable src, AccessType access_type);
    IRVariable sub(IRValue lhs, IRValue rhs, bool set_flags);
    void store_flags(Flags flags);
    
private:
    template <typename T, typename... Args>
    void push(Args... args) {
        basic_block.opcodes.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    u32 next_variable_id{0};
    BasicBlock& basic_block;
};

} // namespace arm