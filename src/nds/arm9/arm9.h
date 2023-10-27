#pragma once

#include <memory>
#include "arm/cpu.h"
#include "nds/arm9/memory.h"
#include "nds/arm9/coprocessor.h"
#include "nds/hardware/irq.h"

namespace nds {

class System;

class ARM9 {
public:
    ARM9(System& system);

    void reset();
    void run(int cycles);
    void select_backend(arm::BackendType backend, bool optimise);
    void direct_boot();
    void firmware_boot();
    bool is_halted();
    ARM9Memory& get_memory() { return memory; }
    ARM9Coprocessor& get_coprocessor() { return coprocessor; }
    IRQ& get_irq() { return irq; }
    arm::CPU& get_cpu() { return *cpu; }

private:
    System& system;
    ARM9Memory memory;
    ARM9Coprocessor coprocessor;
    IRQ irq;
    std::unique_ptr<arm::CPU> cpu;
};

} // namespace nds