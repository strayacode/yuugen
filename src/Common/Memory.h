#pragma once

#include <string.h>
#include "Common/Types.h"

namespace Common {

template <typename T>
T read(void* data, int offset) {
    T return_value = 0;

    memcpy(&return_value, (u8*)data + offset, sizeof(T));

    return return_value;
}

template <typename T>
void write(void* data, int offset, T value) {
    memcpy((u8*)data + offset, &value, sizeof(T));
}

inline bool in_range(u32 start, u32 end, u32 addr) {
    return addr >= start && addr < end;
}

}