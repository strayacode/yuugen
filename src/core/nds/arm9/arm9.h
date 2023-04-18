#pragma once

#include <memory>
#include "core/arm/cpu.h"
#include "core/nds/arm9/memory.h"

namespace core::nds {

class System;

class ARM9 {
public:
    ARM9(System& system);

    void run(int cycles);
    void select_backend(arm::Backend backend);
    void direct_boot();
    ARM9Memory& get_memory() { return memory; }

private:
    ARM9Memory memory;
    std::unique_ptr<arm::CPU> cpu;
};

} // namespace core::nds