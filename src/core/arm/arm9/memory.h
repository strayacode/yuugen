#pragma once

#include <core/arm/memory_base.h>
#include <common/memory_helpers.h>
#include <common/types.h>
#include <common/log.h>

class HW;

class ARM9Memory : public MemoryBase {
public:
    ARM9Memory(HW* hw);

    void Reset();
    void UpdateMemoryMap(u32 low_addr, u32 high_addr);

    auto ReadByte(u32 addr) -> u8 override;
    auto ReadHalf(u32 addr) -> u16 override;
    auto ReadWord(u32 addr) -> u32 override;

    void WriteByte(u32 addr, u8 data) override;
    void WriteHalf(u32 addr, u16 data) override;
    void WriteWord(u32 addr, u32 data) override;

    HW* hw;
};