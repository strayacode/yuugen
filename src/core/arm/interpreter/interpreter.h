#pragma once

#include <array>
#include "core/arm/cpu.h"
#include "core/arm/arch.h"
#include "core/arm/memory.h"
#include "core/arm/coprocessor.h"

namespace core::arm {

class Interpreter : public CPU {
public:
    Interpreter(Arch arch, Memory& memory, Coprocessor& coprocessor);

    void reset() override;
    void run(int cycles) override;
    void jump_to(u32 addr) override;
    void set_mode(Mode mode) override;

private:
    void arm_flush_pipeline();
    void thumb_flush_pipeline();

    Arch arch;
    Memory& memory;
    Coprocessor& coprocessor;
    std::array<u32, 2> pipeline;
};

} // namespace core::arm