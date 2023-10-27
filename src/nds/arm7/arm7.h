#pragma once

#include <memory>
#include "arm/cpu.h"
#include "arm/null_coprocessor.h"
#include "nds/arm7/memory.h"
#include "nds/hardware/irq.h"

namespace nds {

class System;

class ARM7 {
public:
    ARM7(System& system);

    void reset();
    void run(int cycles);
    void select_backend(arm::BackendType backend, bool optimise);
    void direct_boot();
    void firmware_boot();
    bool is_halted();
    ARM7Memory& get_memory() { return memory; }
    arm::NullCoprocessor& get_coprocessor() { return coprocessor; }
    IRQ& get_irq() { return irq; }
    arm::CPU& get_cpu() { return *cpu; }

private:
    System& system;
    ARM7Memory memory;
    arm::NullCoprocessor coprocessor;
    IRQ irq;
    std::unique_ptr<arm::CPU> cpu;
};

} // namespace nds