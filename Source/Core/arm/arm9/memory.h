#pragma once

#include "Common/Types.h"
#include "Core/arm/memory_base.h"

class System;

class ARM9Memory : public MemoryBase {
public:
    ARM9Memory(System& system);

    void Reset();
    void UpdateMemoryMap(u32 low_addr, u32 high_addr);

    u8 ReadByte(u32 addr) override;
    u16 ReadHalf(u32 addr) override;
    u32 ReadWord(u32 addr) override;

    void WriteByte(u32 addr, u8 data) override;
    void WriteHalf(u32 addr, u16 data) override;
    void WriteWord(u32 addr, u32 data) override;

private:
    void build_mmio();

    System& system;
    std::array<u8, 0x8000> bios;
};