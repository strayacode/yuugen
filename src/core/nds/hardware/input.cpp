#include "core/nds/hardware/input.h"

namespace core::nds {

void Input::reset() {
    extkeyin = 0;
}

u16 Input::read_extkeyin() {
    return extkeyin;
}

} // namespace core::nds