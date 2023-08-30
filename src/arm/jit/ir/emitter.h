#pragma once

#include <memory>
#include "arm/jit/basic_block.h"

namespace arm {

class Emitter {
public:
    Emitter(BasicBlock& basic_block);

    void set_carry();
    void clear_carry();

    BasicBlock& basic_block;

private:
    template <typename T, typename... Args>
    void push(Args... args) {
        basic_block.opcodes.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }
};

} // namespace arm