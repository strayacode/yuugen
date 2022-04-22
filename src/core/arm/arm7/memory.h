#pragma once

#include <core/arm/memory_base.h>
#include "Common/memory_helpers.h"
#include "Common/Types.h"
#include "Common/Log.h"

class System;

class ARM7Memory : public MemoryBase {
public:
    ARM7Memory(System& system);

    void Reset();
    void UpdateMemoryMap(u32 low_addr, u32 high_addr);

    u8 ReadByte(u32 addr);
    u16 ReadHalf(u32 addr);
    u32 ReadWord(u32 addr);

    void WriteByte(u32 addr, u8 data) override;
    void WriteHalf(u32 addr, u16 data) override;
    void WriteWord(u32 addr, u32 data) override;

    System& system;
    std::array<u8, 0x4000> bios;
    u8 arm7_wram[0x10000] = {};
};