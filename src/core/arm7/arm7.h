#pragma once

#include <memory>
#include "arm/cpu.h"
#include "core/arm7/memory.h"
#include "core/arm7/coprocessor.h"
#include "core/hardware/irq.h"

namespace core {

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
    IRQ& get_irq() { return irq; }
    arm::CPU& get_cpu() { return *cpu; }

private:
    System& system;
    ARM7Memory memory;
    ARM7Coprocessor coprocessor;
    IRQ irq;
    std::unique_ptr<arm::CPU> cpu;
};

} // namespace core