#pragma once

#include "Common/Types.h"

namespace Common {

template <typename T>
inline int bit_count(T data) {
    int n = sizeof(T) * 8;
    int count = 0;

    for (int i = 0; i < n; i++) {
        if (data & (1 << i)) {
            count++;
        }
    }

    return count;
}

template <typename T, int N>
inline T sign_extend(T value) {
    T mask = static_cast<T>(1) << (N - 1);
    return (value ^ mask) - mask;
}

template <typename T>
inline int num_bits() {
    return sizeof(T) * 8;
}

template <typename T = u32>
inline T rotate_right(u32 value, int n) {
    return (value >> n) | (value << (32 - n));
}

}
