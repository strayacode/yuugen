#include <core/hw/input/input.h>

void Input::Reset() {
    KEYINPUT = 0x3FF;
    // TODO: handle correctly
    EXTKEYIN = 0x7F;
}

void Input::HandleInput(int button, bool pressed) {
    // 0 means key is pressed, 1 means released
    if (pressed) {
        KEYINPUT &= ~(1 << button);
    } else {
        KEYINPUT |= (1 << button);
    }
}