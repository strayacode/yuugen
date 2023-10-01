#include "common/logger.h"
#include "gba/hardware/input.h"

namespace gba {

void Input::reset() {
    keyinput.data = 0x3ff;
    extkeyin = 0x7f;
}

} // namespace gba