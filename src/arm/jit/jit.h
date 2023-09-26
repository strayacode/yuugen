#pragma once

#include <array>
#include <memory>
#include "arm/cpu.h"
#include "arm/arch.h"
#include "arm/memory.h"
#include "arm/coprocessor.h"
#include "arm/decoder.h"
#include "arm/instructions.h"
#include "arm/jit/ir/optimiser.h"
#include "arm/jit/config.h"
#include "arm/jit/backend/backend.h"

namespace arm {

class Jit : public CPU {
public:
    Jit(Arch arch, Memory& memory, Coprocessor& coprocessor, BackendType backend_type);

    void reset() override;
    void run(int cycles) override;
    void update_irq(bool irq) override;
    bool is_halted() override;
    void update_halted(bool halted) override;
    Arch get_arch() override;

    u32 get_gpr(GPR gpr) override;
    u32 get_gpr(GPR gpr, Mode mode) override;
    void set_gpr(GPR gpr, u32 value) override;
    void set_gpr(GPR gpr, Mode mode, u32 value) override;

    StatusRegister get_cpsr() override;
    void set_cpsr(StatusRegister value) override;
    StatusRegister get_spsr(Mode mode) override;
    void set_spsr(Mode mode, StatusRegister value) override;

    u8 read_byte(u32 addr);
    u16 read_half(u32 addr);
    u32 read_word(u32 addr);
    u32 read_word_rotate(u32 addr);

    void write_byte(u32 addr, u8 data);
    void write_half(u32 addr, u16 data);
    void write_word(u32 addr, u32 data);

    void log_state();

    Arch arch;
    Memory& memory;
    Coprocessor& coprocessor;
    Config config;
    
private:
    bool has_spsr(Mode mode);
    u32* get_pointer_to_gpr(GPR gpr, Mode mode);
    StatusRegister* get_pointer_to_cpsr();
    StatusRegister* get_pointer_to_spsr(Mode mode);
    Bank get_bank_from_mode(Mode mode);

    void handle_interrupt();

    bool irq;
    bool halted;

    int cycles_available;
    std::unique_ptr<Backend> backend;
    Optimiser optimiser;
};

} // namespace arm