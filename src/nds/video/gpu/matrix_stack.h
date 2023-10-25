#pragma once

#include <array>
#include "nds/video/gpu/matrix.h"

namespace nds {

template <int N>
class MatrixStack {
public:
    void reset() {
        current.reset();
        stack.fill(Matrix{});

        for (auto& matrix: stack) {
            matrix.reset();
        }

        error = false;
        pointer = 0;
    }

    void push() {
        if (pointer == N) {
            error = true;
            return;
        }

        stack[pointer++] = current;
    }

    void store(int offset) {
        if (N == 1) {
            stack[0] = current;
        } else if (offset < N) {
            stack[offset] = current;
        } else {
            error = true;
        }
    }

    void pop(int offset) {
        pointer -= offset;

        if (pointer < 0) {
            pointer = 0;
            error = true;
        } else if (pointer >= N) {
            pointer = N - 1;
            error = true;
        }

        current = stack[pointer];
    }

    void restore(int offset) {
        if (N == 1) {
            current = stack[0];
        } else if (offset < N) {
            current = stack[offset];
        } else {
            error = true;
        }
    }

    Matrix current;

private:
    std::array<Matrix, N> stack;
    bool error;
    int pointer;
};

} // namespace nds