#pragma once

#include <common/types.h>
#include <string.h>
#include <array>

// this is a base class
// which is used by the arm7
// and arm9 memory classes

class MemoryBase {
    template <typename T>
    auto FastRead(u32 addr) -> T;

    template <typename T>
    void FastWrite(u32 addr, T data);

    virtual auto ReadByte(u32 addr) -> u8 = 0;
    virtual auto ReadHalf(u32 addr) -> u16 = 0;
    virtual auto ReadWord(u32 addr) -> u32 = 0;

    virtual void WriteByte(u32 addr, u8 data) = 0;
    virtual void WriteHalf(u32 addr, u16 data) = 0;
    virtual void WriteWord(u32 addr, u32 data) = 0;

    std::array<u8*, 0x100000> page_table;
};