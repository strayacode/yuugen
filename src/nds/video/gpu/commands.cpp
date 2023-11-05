#include <cassert>
#include "common/logger.h"
#include "common/bits.h"
#include "nds/video/gpu/gpu.h"

namespace nds {

void GPU::set_matrix_mode() {
    const u32 parameter = dequeue_entry().parameter;
    matrix_mode = static_cast<MatrixMode>(common::get_field<0, 2>(parameter));
}

void GPU::pop_current_matrix() {
    const u32 parameter = dequeue_entry().parameter;
    const s8 offset = common::sign_extend<s8, 6>(common::get_field<0, 6>(parameter));

    switch (matrix_mode) {
    case MatrixMode::Projection:
        projection.pop(offset);
        break;
    case MatrixMode::Modelview: case MatrixMode::Simultaneous:
        modelview.pop(offset);
        direction.pop(offset);
        break;
    case MatrixMode::Texture:
        texture.pop(offset);
        break;
    }
}

void GPU::load_unit_matrix() {
    dequeue_entry();

    switch (matrix_mode) {
    case MatrixMode::Projection:
        projection.current.reset();
        break;
    case MatrixMode::Modelview:
        modelview.current.reset();
        break;
    case MatrixMode::Simultaneous:
        modelview.current.reset();
        direction.current.reset();
        break;
    case MatrixMode::Texture:
        texture.current.reset();
        break;
    }
}

void GPU::swap_buffers() {
    const u32 parameter = dequeue_entry().parameter;
    
    // TODO: handle these bits later
    const bool manual_polygon_y_sorting = common::get_bit<0>(parameter);
    const bool w_buffering = common::get_bit<1>(parameter);
    swap_buffers_requested = true;
}

void GPU::set_texture_parameters() {
    const u32 parameter = dequeue_entry().parameter;
    current_polygon.texture_attributes.parameters.data = parameter;
}

void GPU::set_polygon_attributes() {
    // TODO: only set until next BEGIN_VTX command
    const u32 parameter = dequeue_entry().parameter;
    current_polygon.polygon_attributes.data = parameter;
}

void GPU::set_viewport() {
    const u32 parameter = dequeue_entry().parameter;
    viewport.x0 = common::get_field<0, 8>(parameter);
    viewport.y0 = common::get_field<8, 8>(parameter);
    viewport.x1 = common::get_field<16, 8>(parameter);
    viewport.y1 = common::get_field<24, 8>(parameter);

    if (viewport.x0 > viewport.x1 || viewport.y0 > viewport.y1) {
        logger.todo("GPU: handle invalid viewport arguments");
    }
}

void GPU::multiply_4x4() {
    Matrix matrix;

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            matrix.field[y][x] = dequeue_entry().parameter;
        }
    }

    switch (matrix_mode) {
    case MatrixMode::Projection:
        projection.current = multiply_matrix_matrix(matrix, projection.current);
        break;
    case MatrixMode::Modelview: 
        modelview.current = multiply_matrix_matrix(matrix, modelview.current);
        break;
    case MatrixMode::Simultaneous:
        modelview.current = multiply_matrix_matrix(matrix, modelview.current);
        direction.current = multiply_matrix_matrix(matrix, direction.current);
        break;
    case MatrixMode::Texture:
        texture.current = multiply_matrix_matrix(matrix, texture.current);
        break;
    }
}

void GPU::multiply_4x3() {
    Matrix matrix;

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 3; x++) {
            matrix.field[y][x] = dequeue_entry().parameter;
        }
    }

    switch (matrix_mode) {
    case MatrixMode::Projection:
        projection.current = multiply_matrix_matrix(matrix, projection.current);
        break;
    case MatrixMode::Modelview: 
        modelview.current = multiply_matrix_matrix(matrix, modelview.current);
        break;
    case MatrixMode::Simultaneous:
        modelview.current = multiply_matrix_matrix(matrix, modelview.current);
        direction.current = multiply_matrix_matrix(matrix, direction.current);
        break;
    case MatrixMode::Texture:
        texture.current = multiply_matrix_matrix(matrix, texture.current);
        break;
    }
}

void GPU::push_current_matrix() {
    dequeue_entry();

    switch (matrix_mode) {
    case MatrixMode::Projection:
        projection.push();
        break;
    case MatrixMode::Modelview: case MatrixMode::Simultaneous:
        modelview.push();
        direction.push();
        break;
    case MatrixMode::Texture:
        texture.push();
        break;
    }
}

void GPU::multiply_translation() {
    Matrix matrix;

    for (int i = 0; i < 3; i++) {
        matrix.field[3][i] = dequeue_entry().parameter;
    }

    switch (matrix_mode) {
    case MatrixMode::Projection:
        projection.current = multiply_matrix_matrix(matrix, projection.current);
        break;
    case MatrixMode::Modelview: 
        modelview.current = multiply_matrix_matrix(matrix, modelview.current);
        break;
    case MatrixMode::Simultaneous:
        modelview.current = multiply_matrix_matrix(matrix, modelview.current);
        direction.current = multiply_matrix_matrix(matrix, direction.current);
        break;
    case MatrixMode::Texture:
        texture.current = multiply_matrix_matrix(matrix, texture.current);
        break;
    }
}

void GPU::multiply_3x3() {
    Matrix matrix;

    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            matrix.field[y][x] = dequeue_entry().parameter;
        }
    }

    switch (matrix_mode) {
    case MatrixMode::Projection:
        projection.current = multiply_matrix_matrix(matrix, projection.current);
        break;
    case MatrixMode::Modelview: 
        modelview.current = multiply_matrix_matrix(matrix, modelview.current);
        break;
    case MatrixMode::Simultaneous:
        modelview.current = multiply_matrix_matrix(matrix, modelview.current);
        direction.current = multiply_matrix_matrix(matrix, direction.current);
        break;
    case MatrixMode::Texture:
        texture.current = multiply_matrix_matrix(matrix, texture.current);
        break;
    }
}

void GPU::begin_vertex_list() {
    const u32 parameter = dequeue_entry().parameter;
    polygon_type = static_cast<PolygonType>(common::get_field<0, 2>(parameter));
    vertex_count = 0;
    polygon_count = 0;
}

void GPU::set_vertex_colour() {
    const u32 parameter = common::get_field<0, 15>(dequeue_entry().parameter);
    current_vertex.colour = Colour::from_u16(parameter);
}

void GPU::add_vertex16() {
    const u32 parameter1 = dequeue_entry().parameter;
    const u32 parameter2 = dequeue_entry().parameter;

    current_vertex.x = static_cast<s16>(common::get_field<0, 16>(parameter1));
    current_vertex.y = static_cast<s16>(parameter1 >> 16);
    current_vertex.z = static_cast<s16>(common::get_field<0, 16>(parameter2));
    submit_vertex();
}

void GPU::end_vertex_list() {
    dequeue_entry();
}

void GPU::set_shininess() {
    // handle later
    for (int i = 0; i < 32; i++) {
        dequeue_entry();
    }
}

void GPU::set_normal_vector() {
    // handle later
    dequeue_entry();
}

} // namespace nds