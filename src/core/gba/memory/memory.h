#pragma once

#include <array>
#include "common/types.h"
#include "common/log.h"
#include "core/arm/memory_base.h"

class GBA;

class GBAMemory : public MemoryBase {
public:
    GBAMemory(GBA& gba);

    void Reset();
    void UpdateMemoryMap(u32 low_addr, u32 high_addr);

    u8 ReadByte(u32 addr) override;
    u16 ReadHalf(u32 addr) override;
    u32 ReadWord(u32 addr) override;

    void WriteByte(u32 addr, u8 data) override;
    void WriteHalf(u32 addr, u16 data) override;
    void WriteWord(u32 addr, u32 data) override;

private:
    GBA& gba;
    std::array<u8, 0x4000> bios;
    std::array<u8, 0x8000> iwram;
    std::array<u8, 0x40000> ewram;
};