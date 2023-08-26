#pragma once

#include "arm/jit/basic_block.h"

namespace arm {

class Jit;

class Translator {
public:
    Translator(Jit& jit);

    void translate(BasicBlock& basic_block);

private:
    // TODO: add visitor methods for each opcode type (use arm/instructions.h)

    Jit& jit;
};

} // namespace arm