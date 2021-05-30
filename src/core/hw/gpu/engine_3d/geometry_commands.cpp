#include <core/hw/gpu/engine_3d/geometry_engine.h>

void GeometryEngine::CommandSetMatrixMode() {
    // set the matrix mode
    // first fetch the parameter
    u32 parameter = DequeueEntry().parameter;
    matrix_mode = parameter & 0x3;
}

void GeometryEngine::CommandPopCurrentMatrix() {
    u32 parameter = DequeueEntry().parameter;
    u8 stack_offset = parameter & (1 << 5) ? -parameter & 0x3F : parameter & 0x3F;
    u8 new_pointer = coordinate_pointer - stack_offset;

    switch (matrix_mode) {
    case 1: case 2:
        // mode 1 and 2 both have the same operation
        // set the error bit in GXSTAT if there is an underflow or overflow
        if ((new_pointer < 0) || (new_pointer >= 31)) {
            GXSTAT |= (1 << 15);
        } else {
            coordinate_current = coordinate_stack[new_pointer];
            directional_current = directional_stack[new_pointer];

            coordinate_pointer = new_pointer;
        }
        break;
    default:
        log_fatal("handle matrix mode %d", matrix_mode);
    }
}

void GeometryEngine::CommandLoadUnitMatrix() {
    Matrix unit_matrix;
    // set a diagonal row of ones
    for (int i = 0; i < 4; i++) {
        unit_matrix.field[i][i] = 1;
    }

    switch (matrix_mode) {
    case 0:
        // projection
        projection_current = unit_matrix;
        break;
    case 1:
        // coordinate
        coordinate_current = unit_matrix;
        break;
    case 2:
        // coordinate and directional
        coordinate_current = unit_matrix;
        directional_current = unit_matrix;
        break;
    case 3:
        // texture
        texture_current = unit_matrix;
        break;
    default:
        log_fatal("handle matrix mode %d", matrix_mode);
    }

    // also make sure to dequeue the entry even though its doesn't have a parameter, just a command
    DequeueEntry();
}