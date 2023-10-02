#include "nds/video/gpu/gpu.h"

namespace nds {

void GPU::reset() {
    disp3dcnt.data = 0;
}

void GPU::write_disp3dcnt(u32 value, u32 mask) {
    disp3dcnt.data = (disp3dcnt.data & ~mask) | (value & mask);
}

} // namespace nds