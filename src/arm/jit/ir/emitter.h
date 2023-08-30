#pragma once

#include <memory>
#include "arm/jit/basic_block.h"
#include "arm/jit/ir/types.h"

namespace arm {

class Emitter {
public:
    Emitter(BasicBlock& basic_block);

    IRVariable create_variable();

    void set_carry();
    void clear_carry();
    void move(IRValue src, bool set_flags);

    BasicBlock& basic_block;

private:
    template <typename T, typename... Args>
    void push(Args... args) {
        basic_block.opcodes.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    int next_variable_id{0};
};

} // namespace arm