#include <core/hw/gpu/engine_3d/geometry_engine.h>
#include <core/hw/gpu/gpu.h>

void GeometryEngine::SetMatrixMode() {
    u32 parameter = DequeueEntry().parameter;
    matrix_mode = parameter & 0x3;
}

void GeometryEngine::PushCurrentMatrix() {
    DequeueEntry();

    switch (matrix_mode) {
    case 0:
        projection_stack = projection_current;
        UpdateClipMatrix();
        break;
    case 1: case 2:
        if ((modelview_pointer < 0) || (modelview_pointer >= 31)) {
            gxstat |= (1 << 15);
        } else {
            modelview_stack[modelview_pointer] = modelview_current;
            direction_stack[modelview_pointer] = direction_current;
            modelview_pointer++;
            UpdateClipMatrix();
        }
        break;
    case 3:
        texture_stack = texture_current;
        break;
    }
}

void GeometryEngine::PopCurrentMatrix() {
    u32 parameter = DequeueEntry().parameter;
    u8 stack_offset = (s8)((parameter & 0x3F) << 2) >> 2;

    switch (matrix_mode) {
    case 0:
        projection_current = projection_stack;
        UpdateClipMatrix();
        break;
    case 1: case 2:
        modelview_pointer -= stack_offset;

        if ((modelview_pointer < 0) || (modelview_pointer >= 31)) {
            gxstat |= (1 << 15);
        } else {
            modelview_current = modelview_stack[modelview_pointer];
            direction_current = direction_stack[modelview_pointer];
            UpdateClipMatrix();
        }
        break;
    case 3:
        texture_current = texture_stack;
        break;
    }
}

void GeometryEngine::LoadUnitMatrix() {
    DequeueEntry();

    Matrix unit_matrix;

    switch (matrix_mode) {
    case 0:
        projection_current = unit_matrix;
        UpdateClipMatrix();
        break;
    case 1:
        modelview_current = unit_matrix;
        UpdateClipMatrix();
        break;
    case 2:
        modelview_current = unit_matrix;
        direction_current = unit_matrix;
        UpdateClipMatrix();
        break;
    case 3:
        texture_current = unit_matrix;
        break;
    }
}

void GeometryEngine::SwapBuffers() {
    state = GeometryEngineState::Halted;

    // TODO: handle bit 0 and 1 of parameter later
    DequeueEntry();
}

void GeometryEngine::SetTextureParameters() {
    DequeueEntry();
}

void GeometryEngine::SetPolygonAttributes() {
    DequeueEntry();
}

void GeometryEngine::SetViewport() {
    u32 parameter = DequeueEntry().parameter;

    screen_x1 = parameter & 0xFF;
    screen_y1 = (parameter >> 8) & 0xFF;
    screen_x2 = (parameter >> 16) & 0xFF;
    screen_y2 = (parameter >> 24) & 0xFF;
}

void GeometryEngine::Multiply4x4() {
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
        projection_current = MultiplyMatrixMatrix(matrix, projection_current);
        UpdateClipMatrix();
        break;
    case 1: 
        modelview_current = MultiplyMatrixMatrix(matrix, modelview_current);
        UpdateClipMatrix();
        break;
    case 2:
        modelview_current = MultiplyMatrixMatrix(matrix, modelview_current);
        direction_current = MultiplyMatrixMatrix(matrix, projection_current);
        UpdateClipMatrix();
        break;
    case 3:
        texture_current = MultiplyMatrixMatrix(matrix, texture_current);
        break;
    }
}

void GeometryEngine::Multiply4x3() {
    Matrix matrix;

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 3; x++) {
            u32 parameter = DequeueEntry().parameter;
            matrix.field[y][x] = (s32)parameter;
        }
    }

    switch (matrix_mode) {
    case 0:
        projection_current = MultiplyMatrixMatrix(matrix, projection_current);
        UpdateClipMatrix();
        break;
    case 1: 
        modelview_current = MultiplyMatrixMatrix(matrix, modelview_current);
        UpdateClipMatrix();
        break;
    case 2:
        modelview_current = MultiplyMatrixMatrix(matrix, modelview_current);
        direction_current = MultiplyMatrixMatrix(matrix, projection_current);
        UpdateClipMatrix();
        break;
    case 3:
        texture_current = MultiplyMatrixMatrix(matrix, texture_current);
        break;
    }
}

void GeometryEngine::MultiplyTranslation() {
    Matrix matrix;

    for (int i = 0; i < 3; i++) {
        u32 parameter = DequeueEntry().parameter;
        matrix.field[3][i] = (s32)parameter;
    }

    switch (matrix_mode) {
    case 0:
        projection_current = MultiplyMatrixMatrix(matrix, projection_current);
        UpdateClipMatrix();
        break;
    case 1: 
        modelview_current = MultiplyMatrixMatrix(matrix, modelview_current);
        UpdateClipMatrix();
        break;
    case 2:
        modelview_current = MultiplyMatrixMatrix(matrix, modelview_current);
        direction_current = MultiplyMatrixMatrix(matrix, projection_current);
        UpdateClipMatrix();
        break;
    case 3:
        texture_current = MultiplyMatrixMatrix(matrix, texture_current);
        break;
    }
}

void GeometryEngine::Multiply3x3() {
    Matrix matrix;

    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            u32 parameter = DequeueEntry().parameter;
            matrix.field[y][x] = (s32)parameter;
        }
    }

    switch (matrix_mode) {
    case 0:
        projection_current = MultiplyMatrixMatrix(matrix, projection_current);
        UpdateClipMatrix();
        break;
    case 1: 
        modelview_current = MultiplyMatrixMatrix(matrix, modelview_current);
        UpdateClipMatrix();
        break;
    case 2:
        modelview_current = MultiplyMatrixMatrix(matrix, modelview_current);
        direction_current = MultiplyMatrixMatrix(matrix, projection_current);
        UpdateClipMatrix();
        break;
    case 3:
        texture_current = MultiplyMatrixMatrix(matrix, texture_current);
        break;
    }
}

void GeometryEngine::BeginVertexList() {
    // TODO: handle polygon rendering later
    DequeueEntry();
}

void GeometryEngine::EndVertexList() {
    DequeueEntry();
}

void GeometryEngine::SetVertexColour() {
    u32 parameter = DequeueEntry().parameter;

    current_vertex.colour = parameter & 0x7FFF;
}

void GeometryEngine::AddVertex16() {
    u32 parameter1 = DequeueEntry().parameter;
    u32 parameter2 = DequeueEntry().parameter;

    current_vertex.x = (s16)(parameter1 & 0xFFFF);
    current_vertex.y = (s16)(parameter1 >> 16);
    current_vertex.z = (s16)(parameter2 & 0xFFFF);

    AddVertex();
}