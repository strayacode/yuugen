#pragma once

#include <core/arm/memory_base.h>

class ARM7Memory : public MemoryBase {
    void UpdateMemoryMap();

    auto ReadByte(u32 addr) -> u8 override;
    auto ReadHalf(u32 addr) -> u16 override;
    auto ReadWord(u32 addr) -> u32 override;

    void WriteByte(u32 addr, u8 data) override;
    void WriteHalf(u32 addr, u16 data) override;
    void WriteWord(u32 addr, u32 data) override;
};