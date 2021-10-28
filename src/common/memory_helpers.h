#pragma once

#define in_range(lower_bound, size) (addr >= (unsigned)lower_bound) && (addr < (unsigned)(lower_bound + size))

#include <string.h>
#include <stdlib.h>
#include <common/types.h>

template <typename T>
auto read(void* dest) -> T {
    T return_value = 0;

    memcpy(&return_value, (u8*)dest, sizeof(T));

    return return_value;
}

template <typename T>
void write(void* dest, T data) {
    memcpy((u8*)dest, &data, sizeof(T));
}