#include <core/arm/memory_base.h>

template <typename T>
auto MemoryBase::FastRead(u32 addr) -> T {
    addr &= ~(sizeof(T) - 1);

    T return_value = 0;

    int index = addr >> 12;
    int offset = addr & 0xFFF;

    if (page_table[index][offset]) {
        memcpy(&return_value, &page_table[index][offset], sizeof(T));
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

template <typename T>
void MemoryBase::FastWrite(u32 addr, T data) {
    addr &= ~(sizeof(T) - 1);

    int index = addr >> 12;
    int offset = addr & 0xFFF;

    if (page_table[index][offset]) {
        memcpy(&page_table[index][offset], &data, sizeof(T));
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