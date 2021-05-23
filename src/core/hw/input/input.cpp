#include <core/hw/input/input.h>

void Input::Reset() {
    KEYINPUT = 0x3FF;
    EXTKEYIN = 0x7F;

    point.x = 0;
    point.y = 0;
}

void Input::HandleInput(int button, bool pressed) {
    // 0 means key is pressed, 1 means released
    if (pressed) {
        KEYINPUT &= ~(1 << button);
    } else {
        KEYINPUT |= (1 << button);
    }
}

void Input::SetTouch(bool pressed) {
    // 0 means screen is touched, 1 means no
    if (pressed) {
        EXTKEYIN &= ~(1 << 6);
    } else {
        EXTKEYIN |= (1 << 6);
    }
}

void Input::SetPoint(int x, int y) {
    point.x = x;
    point.y = y;
}

auto Input::TouchDown() -> bool {
    return (!(EXTKEYIN & (1 << 6)));
}