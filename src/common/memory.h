#pragma once

#include <cstring>
#include "common/types.h"

namespace common {

template <typename T>
inline T read(void* data, int offset = 0) {
    T return_value{};
    std::memcpy(&return_value, reinterpret_cast<u8*>(data) + offset, sizeof(T));
    return return_value;
}

template <typename T>
inline void write(void* data, T value, int offset = 0) {
    std::memcpy(reinterpret_cast<u8*>(data) + offset, &value, sizeof(T));
}

inline bool in_range(u32 start, u32 end, u32 addr) {
    return addr >= start && addr < end;
}

} // namespace common