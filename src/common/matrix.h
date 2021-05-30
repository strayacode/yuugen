#pragma once

#include <common/types.h>
#include <string.h>

struct Matrix {
    Matrix() {
        // reset the matrix
        for (int i = 0; i < 4; i++) {
            memset(&field[i], 0, 4 * sizeof(s32));
        }
    }

    s32 field[4][4];
};