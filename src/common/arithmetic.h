#pragma once

#define rotate_right(data, shift_amount) ((data) >> (shift_amount)) | ((data) << (32 - (shift_amount)))

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