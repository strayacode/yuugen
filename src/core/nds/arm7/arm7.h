#pragma once

#include <memory>
#include "core/arm/cpu.h"
#include "core/nds/arm7/memory.h"
#include "core/nds/arm7/coprocessor.h"

namespace core::nds {

class System;

class ARM7 {
public:
    ARM7(System& system);

    void reset();
    void run(int cycles);
    void select_backend(arm::Backend backend);
    void direct_boot();
    ARM7Memory& get_memory() { return memory; }
    ARM7Coprocessor& get_coprocessor() { return coprocessor; }

private:
    ARM7Memory memory;
    ARM7Coprocessor coprocessor;
    std::unique_ptr<arm::CPU> cpu;
    System& system;
};

} // namespace core::nds