#pragma once

#include <array>
#include "common/types.h"

namespace nds {

class Matrix {
public:
    Matrix() {
        reset();
    }

    void reset() {
        // initialise to a 4x4 identity matrix in fixed point notation
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (i == j) {
                    field[i][j] = 1 << 12;
                } else {
                    field[i][j] = 0;
                }
            }
        }
    }

    std::array<std::array<s32, 4>, 4> field;
};

} // namespace nds