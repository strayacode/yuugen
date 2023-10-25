#include "common/logger.h"
#include "common/bits.h"
#include "nds/video/gpu/gpu.h"

namespace nds {

void GPU::set_matrix_mode() {
    auto parameter = dequeue_entry().parameter;
    matrix_mode = static_cast<MatrixMode>(common::get_field<0, 2>(parameter));
}

void GPU::pop_current_matrix() {
    auto parameter = dequeue_entry().parameter;
    s8 offset = common::sign_extend<s8, 6>(common::get_field<0, 6>(parameter));

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

} // namespace nds