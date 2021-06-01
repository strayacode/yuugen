#include <core/hw/gpu/engine_3d/geometry_engine.h>
#include <core/core.h>

void GeometryEngine::CommandSetMatrixMode() {
    // set the matrix mode
    // first fetch the parameter
    u32 parameter = DequeueEntry().parameter;
    matrix_mode = parameter & 0x3;
}

void GeometryEngine::CommandPopCurrentMatrix() {
    u32 parameter = DequeueEntry().parameter;
    u8 stack_offset = parameter & (1 << 5) ? -parameter & 0x3F : parameter & 0x3F;
    u8 new_pointer = position_pointer - stack_offset;

    switch (matrix_mode) {
    case 0:
        // projection stack
        if ((projection_pointer > 1) || (projection_pointer < 0)) {
            GXSTAT |= (1 << 15);
        } else {
            projection_pointer--;
            projection_current = projection_stack;
            UpdateClipMatrix();
        }
        break;
    case 1: case 2:
        // mode 1 and 2 both have the same operation
        // set the error bit in GXSTAT if there is an underflow or overflow
        if ((new_pointer < 0) || (new_pointer >= 31)) {
            GXSTAT |= (1 << 15);
        } else {
            position_current = position_stack[new_pointer];
            directional_current = directional_stack[new_pointer];
            position_pointer = new_pointer;
            UpdateClipMatrix();
        }
        break;
    default:
        log_fatal("handle matrix mode %d", matrix_mode);
    }
}

void GeometryEngine::CommandPushCurrentMatrix() {
    // dequeue
    DequeueEntry();

    switch (matrix_mode) {
    case 0:
        // projection stack
        if ((projection_pointer > 1) || (projection_pointer < 0)) {
            GXSTAT |= (1 << 15);
        } else {
            projection_stack = projection_current;
            projection_pointer++;
            UpdateClipMatrix();
        }
        break;
    case 1: case 2:
        // mode 1 and 2 both have the same operation
        if ((position_pointer < 0) || (position_pointer >= 31)) {
            GXSTAT |= (1 << 15);
        } else {
            position_stack[position_pointer] = position_current;
            directional_stack[position_pointer] = directional_current;
            position_pointer++;
            UpdateClipMatrix();
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
        UpdateClipMatrix();
        break;
    case 1:
        // position
        position_current = unit_matrix;
        UpdateClipMatrix();
        break;
    case 2:
        // position and directional
        position_current = unit_matrix;
        directional_current = unit_matrix;
        UpdateClipMatrix();
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

void GeometryEngine::CommandSwapBuffers() {
    // halt the geometry engine
    state = STATE_HALTED;

    // read the parameter
    u32 parameter = DequeueEntry().parameter;

    // TODO: handle bit 0 and 1 of parameter later
}

void GeometryEngine::DoSwapBuffers() {
    // replace the render engines vertex and polygon ram with the geometry engines and empty the geometry engines
    gpu->render_engine.polygon_ram = polygon_ram;
    gpu->render_engine.vertex_ram = vertex_ram;

    // std::fill(std::begin(polygon_ram), std::end(polygon_ram), 0);
    // std::fill(std::begin(vertex_ram), std::end(vertex_ram), 0);

    // unhalt the geometry engine
    state = STATE_RUNNING;
    
    // schedule more interpret commands events with 1 cycle delay
    gpu->core->scheduler.Add(1, InterpretCommandTask);
}

void GeometryEngine::CommandSetTextureParameters() {
    u32 parameter = DequeueEntry().parameter;

    // for now don't do anything when we don't have textures
}

void GeometryEngine::CommandSetPolygonAttributes() {
    u32 parameter = DequeueEntry().parameter;

    // for now don't do anything
}

void GeometryEngine::CommandSetViewport() {
    // set the viewport of the render engines end for when it does rendering ig?
    u32 parameter = DequeueEntry().parameter;

    gpu->render_engine.screen_x1 = parameter & 0xFF;
    gpu->render_engine.screen_y1 = (parameter >> 8) & 0xFF;
    gpu->render_engine.screen_x2 = (parameter >> 16) & 0xFF;
    gpu->render_engine.screen_y2 = (parameter >> 24) & 0xFF;
}

void GeometryEngine::CommandMultiply4x4() {
    // create a new 4x4 matrix, and fill each cell with an s32 number
    Matrix matrix;

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            u32 parameter = DequeueEntry().parameter;
            matrix.field[y][x] = (s32)parameter;
        }
    }

    switch (matrix_mode) {
    case 0:
        // projection
        projection_current = MatrixMultiply(projection_current, matrix);
        UpdateClipMatrix();
        break;
    default:
        log_fatal("handle matrix mode %d", matrix_mode);
    }
}

void GeometryEngine::CommandMultiply4x3() {
    // create a new 4x3 matrix, and fill each cell with an s32 number
    Matrix matrix;

    matrix.field[3][3] = 1;

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 3; x++) {
            u32 parameter = DequeueEntry().parameter;
            matrix.field[y][x] = (s32)parameter;
        }
    }

    // TODO: just check the mode in matrix multiply or smth
    switch (matrix_mode) {
    case 0:
        // projection
        projection_current = MatrixMultiply(projection_current, matrix);
        UpdateClipMatrix();
        break;
    case 2:
        // position and directional
        position_current = MatrixMultiply(position_current, matrix);
        directional_current = MatrixMultiply(directional_current, matrix);
        UpdateClipMatrix();
        break;
    default:
        log_fatal("handle matrix mode %d", matrix_mode);
    }
}

void GeometryEngine::CommandMultiply3x3() {
    // create a new 3x3 matrix, and fill each cell with an s32 number
    Matrix matrix;

    matrix.field[3][3] = 1;

    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            u32 parameter = DequeueEntry().parameter;
            matrix.field[y][x] = (s32)parameter;
        }
    }

    // TODO: just check the mode in matrix multiply or smth
    switch (matrix_mode) {
    case 0:
        // projection
        projection_current = MatrixMultiply(projection_current, matrix);
        UpdateClipMatrix();
        break;
    case 2:
        // position and directional
        position_current = MatrixMultiply(position_current, matrix);
        directional_current = MatrixMultiply(directional_current, matrix);
        UpdateClipMatrix();
        break;
    default:
        log_fatal("handle matrix mode %d", matrix_mode);
    }
}

void GeometryEngine::CommandMultiplyTranslation() {
    Matrix matrix;

    // set up the unit matrix part
    for (int i = 0; i < 4; i++) {
        matrix.field[i][i] = 1;
    }

    // write to lowest row for 3 parameters
    for (int j = 0; j < 3; j++) {
        u32 parameter = DequeueEntry().parameter;
        matrix.field[3][j] = (s32)parameter;
    }

    switch (matrix_mode) {
    case 0:
        // projection
        projection_current = MatrixMultiply(projection_current, matrix);
        UpdateClipMatrix();
        break;
    case 2:
        // position and directional
        position_current = MatrixMultiply(position_current, matrix);
        directional_current = MatrixMultiply(directional_current, matrix);
        UpdateClipMatrix();
        break;
    default:
        log_fatal("handle matrix mode %d", matrix_mode);
    }
}

void GeometryEngine::CommandBeginVertexList() {
    // read in the parameter
    u32 parameter = DequeueEntry().parameter;

    polygon_type = parameter & 0x3;
}

void GeometryEngine::CommandSetVertexColour() {
    // read in the parameter
    u32 parameter = DequeueEntry().parameter;

    vertex_colour = parameter & 0x7FFF;
}

void GeometryEngine::CommandAddVertex16() {
    // read in the parameter
    u32 parameter1 = DequeueEntry().parameter;
    u32 parameter2 = DequeueEntry().parameter;

    Vertex v;
    v.x = parameter1 & 0xFFFF;
    v.y = (parameter1 >> 16) & 0xFFFF;
    v.z = parameter2 & 0xFFFF;

    AddVertex(v);
}

void GeometryEngine::CommandShininess() {
    // handle later lol
    for (int i = 0; i < 32; i++) {
        DequeueEntry();
    }
}

void GeometryEngine::CommandSetTexturePaletteAddress() {
    // handle later lol
    DequeueEntry();
}

void GeometryEngine::CommandLoad4x4() {
    // create a new 4x4 matrix, and fill each cell with an s32 number
    Matrix matrix;

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            u32 parameter = DequeueEntry().parameter;
            matrix.field[y][x] = (s32)parameter;
        }
    }

    switch (matrix_mode) {
    case 0:
        // projection
        projection_current = matrix;
        UpdateClipMatrix();
        break;
    case 1:
        // position
        position_current = matrix;
        UpdateClipMatrix();
        break;
    case 2:
        // position and directional
        position_current = matrix;
        directional_current = matrix;
        UpdateClipMatrix();
        break;
    case 3:
        // texture
        texture_current = matrix;
        break;
    default:
        log_fatal("handle matrix mode %d", matrix_mode);
    }
}