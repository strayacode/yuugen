#include "Core/Hardware/Input.h"

void Input::Reset() {
    keyinput = 0x3FF;
    extkeyin = 0x7F;

    point.x = 0;
    point.y = 0;
}

void Input::build_mmio(MMIO& mmio) {
    mmio.register_mmio<u16>(
        0x04000130,
        mmio.direct_read<u16>(&keyinput, 0x9ff),
        mmio.direct_write<u16>(&keyinput, 0x9ff)
    );
}

void Input::HandleInput(int button, bool pressed) {
    // 0 means key is pressed, 1 means released
    if (pressed) {
        keyinput &= ~(1 << button);
    } else {
        keyinput |= (1 << button);
    }
}

void Input::SetTouch(bool pressed) {
    // 0 means screen is touched, 1 means no
    if (pressed) {
        extkeyin &= ~(1 << 6);
    } else {
        extkeyin |= (1 << 6);
    }
}

void Input::SetPoint(int x, int y) {
    point.x = x;
    point.y = y;
}

bool Input::TouchDown() {
    return (!(extkeyin & (1 << 6)));
}