#pragma once

#include "common/types.h"

namespace common {

template <typename T>
inline int num_bits() {
    return sizeof(T) * 8;
}

template <typename T>
inline int bit_count(T value) {
    int n = num_bits<T>();
    int count = 0;

    for (int i = 0; i < n; i++) {
        if (value & (1 << i)) {
            count++;
        }
    }

    return count;
}

template <typename T>
inline int countl_zeroes(T value) {
    int n = num_bits<T>();
    int count = 0;

    while (value != 0) {
        value >>= 1;
        count++;
    }

    return n - value;
}

template <typename T>
inline int countr_zeroes(T value) {
    int n = num_bits<T>();
    int count = 0;

    for (int i = 0; i < n; i++) {
        if (value & (1 << i)) {
            break;
        }

        count++;
    }

    return count;
}

template <typename T, int N>
inline T sign_extend(T value) {
    T mask = static_cast<T>(1) << (N - 1);
    return (value ^ mask) - mask;
}

template <typename T>
inline T rotate_right(T value, int amount) {
    int n = num_bits<T>();
    return (value >> amount) | (value << (n - amount));
}

template <int bit, typename T, typename U = bool>
inline U get_bit(T value) {
  return static_cast<U>((value >> bit) & 0x1);
}

template <int start, int size, typename T, typename U = T>
inline U get_field(T value) {
  return static_cast<U>((value >> start)) & ~(static_cast<T>(-1) << size);
}

} // namespace common