#pragma once

// credits go to Duality (by fleroviux) for a nice way
// to abstract matrix stacks
#include <common/types.h>
#include <common/matrix.h>

template <int size>
class MatrixStack {
public:
    void Reset() {
        error = false;
        pointer = 0;
    }

    void Push() {
        if (pointer == size) {
            error = true;
            return;
        }

        stack[pointer++] = current;
    }

    void Store(int offset) {
        if (size == 1) {
            stack[0] = current;
        } else if (offset == size) {
            error = true;
        } else {
            stack[offset] = current;
        }
    }

    void Pop(int offset) {
        pointer -= offset;

        if (pointer < 0) {
            pointer = 0;
            error = true;
        } else if (pointer >= size) {
            pointer = size - 1;
            error = true;
        }

        current = stack[pointer];
    }

    void Restore(int offset) {
        if (size == 1) {
            current = stack[0];
            stack[0] = current;
        } else if (offset == size) {
            error = true;
        } else {
            current = stack[offset];
        }
    }

    Matrix stack[size];

    Matrix current;
    bool error;
    int pointer;
};