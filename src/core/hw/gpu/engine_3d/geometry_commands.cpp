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
        projection.Push();
        break;
    case 1: case 2:
        modelview.Push();
        direction.Push();
        break;
    case 3:
        texture.Push();
        break;
    }
}

void GeometryEngine::StoreCurrentMatrix() {
    u32 parameter = DequeueEntry().parameter;
    u32 offset = parameter & 0x1F;

    switch (matrix_mode) {
    case 0:
        projection.Store(offset);
        break;
    case 1: case 2:
        modelview.Store(offset);
        direction.Store(offset);
        break;
    case 3:
        texture.Store(offset);
        break;
    }
}

void GeometryEngine::PopCurrentMatrix() {
    u32 parameter = DequeueEntry().parameter;
    s32 offset = ((s8)(parameter & 0x3F) << 26) >> 26;

    switch (matrix_mode) {
    case 0:
        projection.Pop(offset);
        UpdateClipMatrix();
        break;
    case 1: case 2:
        modelview.Pop(offset);
        direction.Pop(offset);
        UpdateClipMatrix();
        break;
    case 3:
        texture.Pop(offset);
        break;
    }
}

void GeometryEngine::RestoreCurrentMatrix() {
    u32 parameter = DequeueEntry().parameter;
    u32 offset = parameter & 0x1F;

    switch (matrix_mode) {
    case 0:
        projection.Restore(offset);
        UpdateClipMatrix();
        break;
    case 1: case 2:
        modelview.Restore(offset);
        direction.Restore(offset);
        UpdateClipMatrix();
        break;
    case 3:
        texture.Restore(offset);
        break;
    }
}

void GeometryEngine::LoadUnitMatrix() {
    DequeueEntry();

    Matrix unit_matrix;

    switch (matrix_mode) {
    case 0:
        projection.current = unit_matrix;
        UpdateClipMatrix();
        break;
    case 1:
        modelview.current = unit_matrix;
        UpdateClipMatrix();
        break;
    case 2:
        modelview.current = unit_matrix;
        direction.current = unit_matrix;
        UpdateClipMatrix();
        break;
    case 3:
        texture.current = unit_matrix;
        break;
    }
}

void GeometryEngine::Load4x4() {
    Matrix matrix;

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            u32 parameter = DequeueEntry().parameter;
            matrix.field[y][x] = (s32)parameter;
        }
    }

    switch (matrix_mode) {
    case 0:
        projection.current = matrix;
        UpdateClipMatrix();
        break;
    case 1:
        modelview.current = matrix;
        UpdateClipMatrix();
        break;
    case 2:
        modelview.current = matrix;
        direction.current = matrix;
        UpdateClipMatrix();
        break;
    case 3:
        texture.current = matrix;
        break;
    }
}

void GeometryEngine::Load4x3() {
    Matrix matrix;

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 3; x++) {
            u32 parameter = DequeueEntry().parameter;
            matrix.field[y][x] = (s32)parameter;
        }
    }

    switch (matrix_mode) {
    case 0:
        projection.current = matrix;
        UpdateClipMatrix();
        break;
    case 1:
        modelview.current = matrix;
        UpdateClipMatrix();
        break;
    case 2:
        modelview.current = matrix;
        direction.current = matrix;
        UpdateClipMatrix();
        break;
    case 3:
        texture.current = matrix;
        break;
    }
}

void GeometryEngine::SwapBuffers() {
    // TODO: handle bit 0 and 1 of parameter later
    DequeueEntry();
    DoSwapBuffers();
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
        projection.current = MultiplyMatrixMatrix(matrix, projection.current);
        UpdateClipMatrix();
        break;
    case 1: 
        modelview.current = MultiplyMatrixMatrix(matrix, modelview.current);
        UpdateClipMatrix();
        break;
    case 2:
        modelview.current = MultiplyMatrixMatrix(matrix, modelview.current);
        direction.current = MultiplyMatrixMatrix(matrix, direction.current);
        UpdateClipMatrix();
        break;
    case 3:
        texture.current = MultiplyMatrixMatrix(matrix, texture.current);
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
        projection.current = MultiplyMatrixMatrix(matrix, projection.current);
        UpdateClipMatrix();
        break;
    case 1: 
        modelview.current = MultiplyMatrixMatrix(matrix, modelview.current);
        UpdateClipMatrix();
        break;
    case 2:
        modelview.current = MultiplyMatrixMatrix(matrix, modelview.current);
        direction.current = MultiplyMatrixMatrix(matrix, direction.current);
        UpdateClipMatrix();
        break;
    case 3:
        texture.current = MultiplyMatrixMatrix(matrix, texture.current);
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
        projection.current = MultiplyMatrixMatrix(matrix, projection.current);
        UpdateClipMatrix();
        break;
    case 1: 
        modelview.current = MultiplyMatrixMatrix(matrix, modelview.current);
        UpdateClipMatrix();
        break;
    case 2:
        modelview.current = MultiplyMatrixMatrix(matrix, modelview.current);
        direction.current = MultiplyMatrixMatrix(matrix, direction.current);
        UpdateClipMatrix();
        break;
    case 3:
        texture.current = MultiplyMatrixMatrix(matrix, texture.current);
        break;
    }
}

void GeometryEngine::MultiplyScale() {
    Matrix matrix;

    for (int i = 0; i < 3; i++) {
        u32 parameter = DequeueEntry().parameter;
        matrix.field[i][i] = (s32)parameter;
    }

    switch (matrix_mode) {
    case 0:
        projection.current = MultiplyMatrixMatrix(matrix, projection.current);
        UpdateClipMatrix();
        break;
    case 1: case 2:
        modelview.current = MultiplyMatrixMatrix(matrix, modelview.current);
        UpdateClipMatrix();
        break;
    case 3:
        texture.current = MultiplyMatrixMatrix(matrix, texture.current);
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
        projection.current = MultiplyMatrixMatrix(matrix, projection.current);
        UpdateClipMatrix();
        break;
    case 1: 
        modelview.current = MultiplyMatrixMatrix(matrix, modelview.current);
        UpdateClipMatrix();
        break;
    case 2:
        modelview.current = MultiplyMatrixMatrix(matrix, modelview.current);
        direction.current = MultiplyMatrixMatrix(matrix, direction.current);
        UpdateClipMatrix();
        break;
    case 3:
        texture.current = MultiplyMatrixMatrix(matrix, texture.current);
        break;
    }
}

void GeometryEngine::BeginVertexList() {
    // TODO: handle polygon rendering later
    DequeueEntry();

    vertex_ram_size = 0;
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

void GeometryEngine::AddVertex10() {
    u32 parameter = DequeueEntry().parameter;

    current_vertex.x = (s16)((parameter & 0x000003FF) << 6);
    current_vertex.y = (s16)((parameter & 0x000FFC00) >> 4);
    current_vertex.z = (s16)((parameter & 0x3FF00000) >> 14);

    AddVertex();
}

void GeometryEngine::SetShininess() {
    // handle later
    for (int i = 0; i < 32; i++) {
        DequeueEntry();
    }
}

void GeometryEngine::SetTexturePaletteAddress() {
    // handle later
    DequeueEntry();
}

void GeometryEngine::SetLightVector() {
    // handle later
    DequeueEntry();
}

void GeometryEngine::SetDiffuseAmbientReflect() {
    // handle later
    DequeueEntry();
}

void GeometryEngine::SetSpecularReflectEmission() {
    // handle later
    DequeueEntry();
}

void GeometryEngine::SetLightColour() {
    // handle later
    DequeueEntry();
}

void GeometryEngine::SetTextureCoordinates() {
    // handle later
    DequeueEntry();
}

void GeometryEngine::SetRelativeVertexCoordinates() {
    u32 parameter = DequeueEntry().parameter;

    current_vertex.x += ((s16)((parameter & 0x000003FF) << 6) / 8) >> 3;
    current_vertex.y += ((s16)((parameter & 0x000FFC00) >> 4) / 8) >> 3;
    current_vertex.z += ((s16)((parameter & 0x3FF00000) >> 14) / 8) >> 3;

    AddVertex();
}

void GeometryEngine::SetNormalVector() {
    // handle later
    DequeueEntry();
}

void GeometryEngine::SetVertexXY() {
    u32 parameter = DequeueEntry().parameter;

    current_vertex.x = (s16)(parameter & 0xFFFF);
    current_vertex.y = (s16)(parameter >> 16);

    AddVertex();
}

void GeometryEngine::SetVertexXZ() {
    u32 parameter = DequeueEntry().parameter;

    current_vertex.x = (s16)(parameter & 0xFFFF);
    current_vertex.z = (s16)(parameter >> 16);

    AddVertex();
}

void GeometryEngine::SetVertexYZ() {
    u32 parameter = DequeueEntry().parameter;

    current_vertex.y = (s16)(parameter & 0xFFFF);
    current_vertex.z = (s16)(parameter >> 16);

    AddVertex();
}

void GeometryEngine::BoxTest() {
    DequeueEntry();
    DequeueEntry();
    DequeueEntry();
}