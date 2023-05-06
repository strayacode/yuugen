#include "core/hardware/input.h"

namespace core {

void Input::reset() {
    extkeyin = 0;
}

u16 Input::read_extkeyin() {
    return extkeyin;
}

} // namespace core