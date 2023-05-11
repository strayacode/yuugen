#include "core/hardware/input.h"

namespace core {

void Input::reset() {
    keyinput = 0x3ff;
    extkeyin = 0x7f;
}

u16 Input::read_keyinput() {
    return keyinput & 0x3ff;
}

u16 Input::read_extkeyin() {
    return extkeyin;
}

} // namespace core