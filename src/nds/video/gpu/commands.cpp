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

} // namespace nds