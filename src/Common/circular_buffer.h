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

    bool Empty() {
        return items == 0;
    }

    bool Full() {
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

    T Pop() {
        T data = buffer[head_index];

        if (!Empty()) {
            head_index = (head_index + 1) % size;
            items--;
        }
        
        return data;
    }

    T Front() {
        return buffer[head_index];
    }

    int Size() {
        return items;
    }

private:
    int head_index;
    int tail_index;
    int items;
    std::array<T, size> buffer;
};