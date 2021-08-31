#pragma once

#include <array>

template <typename T, int size>
class CircularBuffer {
public:
    CircularBuffer() {
        head_index = 0;
        tail_index = 0;
        items = 0;
        buffer = {};
    }

    auto Empty() -> bool {
        return items == 0;
    }

    auto Full() -> bool {
        return items == size;
    }

    void Push(T data) {
        if (Full()) {
            return;
        }

        buffer[tail_index] = data;
        tail_index = (tail_index + 1) % size;
        items++;
    }

    auto Pop() -> T {
        T data = buffer[head_index];

        if (!Empty()) {
            head_index = (head_index + 1) % size;
            items--;
        }
        
        return data;
    }

    auto Front() -> T {
        return buffer[head_index];
    }

    auto Size() -> int {
        return items;
    }

private:
    int head_index;
    int tail_index;
    int items;
    std::array<T, size> buffer;
};