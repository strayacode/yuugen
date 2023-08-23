#pragma once

#include <array>

namespace common {

template <typename T, int size>
class RingBuffer {
public:
    RingBuffer() {
        reset();
    }

    bool is_empty() {
        return items == 0;
    }

    bool is_full() {
        return items == size;
    }

    void push(T data) {
        if (is_full()) {
            return;
        }

        buffer[tail_index] = data;
        tail_index = (tail_index + 1) % size;
        items++;
    }

    T pop() {
        T data = buffer[head_index];

        if (!is_empty()) {
            head_index = (head_index + 1) % size;
            items--;
        }
        
        return data;
    }

    T get_front() {
        return buffer[head_index];
    }

    int get_size() {
        return items;
    }

    void reset() {
        head_index = 0;
        tail_index = 0;
        items = 0;
        buffer = {};
    }

private:
    int head_index;
    int tail_index;
    int items;
    std::array<T, size> buffer;
};

} // namespace common