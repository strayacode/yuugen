#pragma once

#include "common/types.h"
#include "core/arm/memory.h"

namespace core::nds {

class System;

class ARM7Memory : public arm::Memory {
public:
    ARM7Memory(System& system);

    u8 system_read_byte(u32 addr) override;
    u16 system_read_half(u32 addr) override;
    u32 system_read_word(u32 addr) override;

    void system_write_byte(u32 addr, u8 value) override;
    void system_write_half(u32 addr, u16 value) override;
    void system_write_word(u32 addr, u32 value) override;

private:
    System& system;
};

} // namespace core::nds