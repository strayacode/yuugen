#include "VideoCommon/Renderer3D.h"

void Renderer3D::SetMatrixMode() {
    u32 parameter = dequeue_entry().parameter;
    matrix_mode = static_cast<MatrixMode>(parameter & 0x3);
}

void Renderer3D::PushCurrentMatrix() {
    dequeue_entry();

    switch (matrix_mode) {
    case MatrixMode::Projection:
        projection.Push();
        break;
    case MatrixMode::Modelview: case MatrixMode::Simultaneous:
        modelview.Push();
        direction.Push();
        break;
    case MatrixMode::Texture:
        texture.Push();
        break;
    }
}

void Renderer3D::PopCurrentMatrix() {
    u32 parameter = dequeue_entry().parameter;
    s8 offset = ((s8)(parameter & 0x3F) << 2) >> 2;

    switch (matrix_mode) {
    case MatrixMode::Projection:
        projection.Pop(offset);
        update_clip_matrix();
        break;
    case MatrixMode::Modelview: case MatrixMode::Simultaneous:
        modelview.Pop(offset);
        direction.Pop(offset);
        update_clip_matrix();
        break;
    case MatrixMode::Texture:
        texture.Pop(offset);
        break;
    }
}

void Renderer3D::StoreCurrentMatrix() {
    u32 parameter = dequeue_entry().parameter;
    u32 offset = parameter & 0x1F;

    switch (matrix_mode) {
    case MatrixMode::Projection:
        projection.Store(offset);
        break;
    case MatrixMode::Modelview: case MatrixMode::Simultaneous:
        modelview.Store(offset);
        direction.Store(offset);
        break;
    case MatrixMode::Texture:
        texture.Store(offset);
        break;
    }
}

void Renderer3D::RestoreCurrentMatrix() {
    u32 parameter = dequeue_entry().parameter;
    u32 offset = parameter & 0x1F;

    switch (matrix_mode) {
    case MatrixMode::Projection:
        projection.Restore(offset);
        update_clip_matrix();
        break;
    case MatrixMode::Modelview: case MatrixMode::Simultaneous:
        modelview.Restore(offset);
        direction.Restore(offset);
        update_clip_matrix();
        break;
    case MatrixMode::Texture:
        texture.Restore(offset);
        break;
    }
}

void Renderer3D::LoadUnitMatrix() {
    dequeue_entry();

    switch (matrix_mode) {
    case MatrixMode::Projection:
        projection.current = Matrix();
        update_clip_matrix();
        break;
    case MatrixMode::Modelview:
        modelview.current = Matrix();
        update_clip_matrix();
        break;
    case MatrixMode::Simultaneous:
        modelview.current = Matrix();
        direction.current = Matrix();
        update_clip_matrix();
        break;
    case MatrixMode::Texture:
        texture.current = Matrix();
        break;
    }
}

void Renderer3D::Load4x4() {
    Matrix matrix;

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            matrix.field[y][x] = dequeue_entry().parameter;
        }
    }

    switch (matrix_mode) {
    case MatrixMode::Projection:
        projection.current = matrix;
        update_clip_matrix();
        break;
    case MatrixMode::Modelview:
        modelview.current = matrix;
        update_clip_matrix();
        break;
    case MatrixMode::Simultaneous:
        modelview.current = matrix;
        direction.current = matrix;
        update_clip_matrix();
        break;
    case MatrixMode::Texture:
        texture.current = matrix;
        break;
    }
}

void Renderer3D::Load4x3() {
    Matrix matrix;

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 3; x++) {
            matrix.field[y][x] = dequeue_entry().parameter;
        }
    }

    switch (matrix_mode) {
    case MatrixMode::Projection:
        projection.current = matrix;
        update_clip_matrix();
        break;
    case MatrixMode::Modelview:
        modelview.current = matrix;
        update_clip_matrix();
        break;
    case MatrixMode::Simultaneous:
        modelview.current = matrix;
        direction.current = matrix;
        update_clip_matrix();
        break;
    case MatrixMode::Texture:
        texture.current = matrix;
        break;
    }
}

void Renderer3D::SwapBuffers() {
    // TODO: handle bit 0 and 1 of parameter later
    u32 parameter = dequeue_entry().parameter;
    w_buffering = (parameter >> 1) & 0x1;

    do_swap_buffers();
}

void Renderer3D::SetTextureParameters() {
    u32 parameter = dequeue_entry().parameter;
    texture_attributes.parameters = parameter;
}

void Renderer3D::set_polygon_attributes() {
    u32 parameter = dequeue_entry().parameter;
    polygon_attributes = parameter;
}

void Renderer3D::SetViewport() {
    u32 parameter = dequeue_entry().parameter;

    screen_x1 = parameter & 0xFF;
    screen_y1 = (parameter >> 8) & 0xFF;
    screen_x2 = (parameter >> 16) & 0xFF;
    screen_y2 = (parameter >> 24) & 0xFF;
}

void Renderer3D::Multiply4x4() {
    // create a new 4x4 matrix, and fill each cell with an s32 number
    Matrix matrix;

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            matrix.field[y][x] = dequeue_entry().parameter;
        }
    }

    switch (matrix_mode) {
    case MatrixMode::Projection:
        projection.current = MultiplyMatrixMatrix(matrix, projection.current);
        update_clip_matrix();
        break;
    case MatrixMode::Modelview: 
        modelview.current = MultiplyMatrixMatrix(matrix, modelview.current);
        update_clip_matrix();
        break;
    case MatrixMode::Simultaneous:
        modelview.current = MultiplyMatrixMatrix(matrix, modelview.current);
        direction.current = MultiplyMatrixMatrix(matrix, direction.current);
        update_clip_matrix();
        break;
    case MatrixMode::Texture:
        texture.current = MultiplyMatrixMatrix(matrix, texture.current);
        break;
    }
}

void Renderer3D::Multiply4x3() {
    Matrix matrix;

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 3; x++) {
            matrix.field[y][x] = dequeue_entry().parameter;
        }
    }

    switch (matrix_mode) {
    case MatrixMode::Projection:
        projection.current = MultiplyMatrixMatrix(matrix, projection.current);
        update_clip_matrix();
        break;
    case MatrixMode::Modelview: 
        modelview.current = MultiplyMatrixMatrix(matrix, modelview.current);
        update_clip_matrix();
        break;
    case MatrixMode::Simultaneous:
        modelview.current = MultiplyMatrixMatrix(matrix, modelview.current);
        direction.current = MultiplyMatrixMatrix(matrix, direction.current);
        update_clip_matrix();
        break;
    case MatrixMode::Texture:
        texture.current = MultiplyMatrixMatrix(matrix, texture.current);
        break;
    }
}

void Renderer3D::MultiplyTranslation() {
    Matrix matrix;

    for (int i = 0; i < 3; i++) {
        matrix.field[3][i] = dequeue_entry().parameter;
    }

    switch (matrix_mode) {
    case MatrixMode::Projection:
        projection.current = MultiplyMatrixMatrix(matrix, projection.current);
        update_clip_matrix();
        break;
    case MatrixMode::Modelview: 
        modelview.current = MultiplyMatrixMatrix(matrix, modelview.current);
        update_clip_matrix();
        break;
    case MatrixMode::Simultaneous:
        modelview.current = MultiplyMatrixMatrix(matrix, modelview.current);
        direction.current = MultiplyMatrixMatrix(matrix, direction.current);
        update_clip_matrix();
        break;
    case MatrixMode::Texture:
        texture.current = MultiplyMatrixMatrix(matrix, texture.current);
        break;
    }
}

void Renderer3D::MultiplyScale() {
    Matrix matrix;

    for (int i = 0; i < 3; i++) {
        matrix.field[i][i] = dequeue_entry().parameter;
    }

    switch (matrix_mode) {
    case MatrixMode::Projection:
        projection.current = MultiplyMatrixMatrix(matrix, projection.current);
        update_clip_matrix();
        break;
    case MatrixMode::Modelview: case MatrixMode::Simultaneous:
        modelview.current = MultiplyMatrixMatrix(matrix, modelview.current);
        update_clip_matrix();
        break;
    case MatrixMode::Texture:
        texture.current = MultiplyMatrixMatrix(matrix, texture.current);
        break;
    }
}

void Renderer3D::Multiply3x3() {
    Matrix matrix;

    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            matrix.field[y][x] = dequeue_entry().parameter;
        }
    }

    switch (matrix_mode) {
    case MatrixMode::Projection:
        projection.current = MultiplyMatrixMatrix(matrix, projection.current);
        update_clip_matrix();
        break;
    case MatrixMode::Modelview: 
        modelview.current = MultiplyMatrixMatrix(matrix, modelview.current);
        update_clip_matrix();
        break;
    case MatrixMode::Simultaneous:
        modelview.current = MultiplyMatrixMatrix(matrix, modelview.current);
        direction.current = MultiplyMatrixMatrix(matrix, direction.current);
        update_clip_matrix();
        break;
    case MatrixMode::Texture:
        texture.current = MultiplyMatrixMatrix(matrix, texture.current);
        break;
    }
}

void Renderer3D::BeginVertexList() {
    // TODO: handle polygon rendering later
    u32 parameter = dequeue_entry().parameter;
    polygon_type = static_cast<PolygonType>(parameter & 0x3);
    vertex_count = 0;
}

void Renderer3D::EndVertexList() {
    dequeue_entry();
}

void Renderer3D::SetVertexColour() {
    u32 parameter = dequeue_entry().parameter & 0x7FFF;
    
    current_vertex.colour = Colour::from_u16(parameter);
}

void Renderer3D::AddVertex16() {
    u32 parameter1 = dequeue_entry().parameter;
    u32 parameter2 = dequeue_entry().parameter;

    current_vertex.x = (s16)(parameter1 & 0xFFFF);
    current_vertex.y = (s16)(parameter1 >> 16);
    current_vertex.z = (s16)(parameter2 & 0xFFFF);

    add_vertex();
}

void Renderer3D::AddVertex10() {
    u32 parameter = dequeue_entry().parameter;

    current_vertex.x = (s16)((parameter & 0x000003FF) << 6);
    current_vertex.y = (s16)((parameter & 0x000FFC00) >> 4);
    current_vertex.z = (s16)((parameter & 0x3FF00000) >> 14);

    add_vertex();
}

void Renderer3D::SetShininess() {
    // handle later
    for (int i = 0; i < 32; i++) {
        dequeue_entry();
    }
}

void Renderer3D::SetTexturePaletteAddress() {
    u32 parameter = dequeue_entry().parameter;
    texture_attributes.palette_base = parameter & 0x1FFF;
}

void Renderer3D::SetLightVector() {
    // handle later
    dequeue_entry();
}

void Renderer3D::SetDiffuseAmbientReflect() {
    // handle later
    dequeue_entry();
}

void Renderer3D::SetSpecularReflectEmission() {
    // handle later
    dequeue_entry();
}

void Renderer3D::SetLightColour() {
    // handle later
    dequeue_entry();
}

void Renderer3D::SetTextureCoordinates() {
    u32 parameter = dequeue_entry().parameter;

    s16 s = static_cast<s16>(parameter & 0xFFFF);
    s16 t = static_cast<s16>(parameter >> 16);

    u8 transformation_mode = (texture_attributes.parameters >> 30) & 0x3;
    
    if (transformation_mode == 1) {
        current_vertex.s = ((s * texture.current.field[0][0]) + (t * texture.current.field[1][0]) + texture.current.field[2][0] + texture.current.field[3][0]) >> 12;
        current_vertex.t = ((s * texture.current.field[0][1]) + (t * texture.current.field[1][1]) + texture.current.field[2][1] + texture.current.field[3][1]) >> 12;
    } else if (transformation_mode == 0) {
        current_vertex.s = s;
        current_vertex.t = t;
    } else {
        log_fatal("Renderer3D: handle transformation mode %d", transformation_mode);
    }
}

void Renderer3D::SetRelativeVertexCoordinates() {
    u32 parameter = dequeue_entry().parameter;

    current_vertex.x += ((s16)((parameter & 0x000003FF) << 6) / 8) >> 3;
    current_vertex.y += ((s16)((parameter & 0x000FFC00) >> 4) / 8) >> 3;
    current_vertex.z += ((s16)((parameter & 0x3FF00000) >> 14) / 8) >> 3;

    add_vertex();
}

void Renderer3D::SetNormalVector() {
    // handle later
    dequeue_entry();
}

void Renderer3D::SetVertexXY() {
    u32 parameter = dequeue_entry().parameter;

    current_vertex.x = (s16)(parameter & 0xFFFF);
    current_vertex.y = (s16)(parameter >> 16);

    add_vertex();
}

void Renderer3D::SetVertexXZ() {
    u32 parameter = dequeue_entry().parameter;

    current_vertex.x = (s16)(parameter & 0xFFFF);
    current_vertex.z = (s16)(parameter >> 16);

    add_vertex();
}

void Renderer3D::SetVertexYZ() {
    u32 parameter = dequeue_entry().parameter;

    current_vertex.y = (s16)(parameter & 0xFFFF);
    current_vertex.z = (s16)(parameter >> 16);

    add_vertex();
}

void Renderer3D::BoxTest() {
    dequeue_entry();
    dequeue_entry();
    dequeue_entry();
}