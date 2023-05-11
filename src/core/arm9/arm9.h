#pragma once

#include <memory>
#include "arm/cpu.h"
#include "core/arm9/memory.h"
#include "core/arm9/coprocessor.h"

namespace core {

class System;

class ARM9 {
public:
    ARM9(System& system);

    void reset();
    void run(int cycles);
    void select_backend(arm::Backend backend);
    void direct_boot();
    ARM9Memory& get_memory() { return memory; }
    ARM9Coprocessor& get_coprocessor() { return coprocessor; }

private:
    System& system;
    ARM9Memory memory;
    ARM9Coprocessor coprocessor;
    std::unique_ptr<arm::CPU> cpu;
};

} // namespace core