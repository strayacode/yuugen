#pragma once

#include <array>
#include "arm/cpu.h"
#include "arm/arch.h"
#include "arm/memory.h"
#include "arm/coprocessor.h"
#include "arm/decoder.h"
#include "arm/instructions.h"

namespace arm {

class Jit : public CPU {
public:
    Jit(Arch arch, Memory& memory, Coprocessor& coprocessor);

    void reset() override;
    void run(int cycles) override;
    void flush_pipeline() override;
    void set_mode(Mode mode) override;
    void update_irq(bool irq) override;
    bool is_halted() override;
    void update_halted(bool halted) override;
    Arch get_arch() override;

private:
    Bank get_bank(Mode mode);

    u16 code_read_half(u32 addr);
    u32 code_read_word(u32 addr);
    u8 read_byte(u32 addr);
    u16 read_half(u32 addr);
    u32 read_word(u32 addr);
    u32 read_word_rotate(u32 addr);

    void write_byte(u32 addr, u8 data);
    void write_half(u32 addr, u16 data);
    void write_word(u32 addr, u32 data);

    Arch arch;
    Memory& memory;
    Coprocessor& coprocessor;
    bool irq;
    bool halted;
};

} // namespace arm