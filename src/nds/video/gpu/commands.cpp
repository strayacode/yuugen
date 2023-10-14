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
    logger.todo("pop from a matrix stack");
}

} // namespace nds