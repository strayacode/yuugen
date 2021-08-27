#pragma once

#include <common/types.h>
#include <string.h>

class Matrix {
public:
    Matrix() {
        // reset the matrix
        // initialise it as a 4x4 identity matrix
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

    s32 field[4][4];
};