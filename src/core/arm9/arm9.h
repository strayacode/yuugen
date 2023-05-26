#pragma once

#include <memory>
#include "arm/cpu.h"
#include "core/arm9/memory.h"
#include "core/arm9/coprocessor.h"
#include "core/hardware/irq.h"

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
    IRQ& get_irq() { return irq; }

private:
    System& system;
    ARM9Memory memory;
    ARM9Coprocessor coprocessor;
    IRQ irq;
    std::unique_ptr<arm::CPU> cpu;
};

} // namespace core