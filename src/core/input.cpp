#include <core/input.h>

void Input::Reset() {
    KEYINPUT = 0x3FF;
}

void Input::HandleInput(int button, bool pressed) {
    // 0 means key is pressed, 1 means released
    if (pressed) {
        KEYINPUT &= ~(1 << button);
    } else {
        KEYINPUT |= (1 << button);
    }
}