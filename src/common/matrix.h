#pragma once

#include <common/types.h>
#include <string.h>

struct Matrix {
    Matrix() {
        // reset the matrix
        // initialise it as a 4x4 unit matrix
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