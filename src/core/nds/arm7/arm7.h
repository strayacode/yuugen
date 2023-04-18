#pragma once

#include <memory>
#include "core/arm/cpu.h"
#include "core/nds/arm7/memory.h"

namespace core::nds {

class System;

class ARM7 {
public:
    ARM7(System& system);

    void run(int cycles);
    void select_backend(arm::Backend backend);
    void direct_boot();
    ARM7Memory& get_memory() { return memory; }

private:
    ARM7Memory memory;
    std::unique_ptr<arm::CPU> cpu;
};

} // namespace core::nds