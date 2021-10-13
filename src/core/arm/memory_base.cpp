#include <core/arm/memory_base.h>

template auto MemoryBase::FastRead(u32 addr) -> u8;
template auto MemoryBase::FastRead(u32 addr) -> u16;
template auto MemoryBase::FastRead(u32 addr) -> u32;
template <typename T>
auto MemoryBase::FastRead(u32 addr) -> T {
    addr &= ~(sizeof(T) - 1);

    T return_value = 0;

    int index = addr >> 12;
    int offset = addr & 0xFFF;

    if (read_page_table[index]) {
        memcpy(&return_value, &read_page_table[index][offset], sizeof(T));
    } else {
        if constexpr (sizeof(T) == 1) {
            return ReadByte(addr);
        } else if constexpr (sizeof(T) == 2) {
            return ReadHalf(addr);
        } else {
            return ReadWord(addr);
        }
    }

    return return_value;
}

template void MemoryBase::FastWrite(u32 addr, u8 data);
template void MemoryBase::FastWrite(u32 addr, u16 data);
template void MemoryBase::FastWrite(u32 addr, u32 data);
template <typename T>
void MemoryBase::FastWrite(u32 addr, T data) {
    addr &= ~(sizeof(T) - 1);

    int index = addr >> 12;
    int offset = addr & 0xFFF;

    if (write_page_table[index]) {
        memcpy(&write_page_table[index][offset], &data, sizeof(T));
    } else {
        if (IsMMIOAddress(addr)) {
            mmio.Write(addr, data);
        } else {
            if constexpr (sizeof(T) == 1) {
                WriteByte(addr, data);
            } else if constexpr (sizeof(T) == 2) {
                WriteHalf(addr, data);
            } else {
                WriteWord(addr, data);
            }
        }
    }
}

bool MemoryBase::IsMMIOAddress(u32 addr) {
    return (addr >> 24) == 0x04;
}