#include "common/logger.h"
#include "core/hardware/input.h"

namespace core {

void Input::reset() {
    point.x = 0;
    point.y = 0;
    keyinput.data = 0x3ff;
    extkeyin = 0x7f;
}

void Input::handle_input(InputEvent event, bool pressed) {
    switch (event) {
    case InputEvent::A:
        keyinput.a = !pressed;
        break;
    case InputEvent::B:
        keyinput.b = !pressed;
        break;
    case InputEvent::Start:
        keyinput.start = !pressed;
        break;
    case InputEvent::Select:
        keyinput.select = !pressed;
        break;
    case InputEvent::Left:
        keyinput.left = !pressed;
        break;
    case InputEvent::Right:
        keyinput.right = !pressed;
        break;
    case InputEvent::Up:
        keyinput.up = !pressed;
        break;
    case InputEvent::Down:
        keyinput.down = !pressed;
        break;
    case InputEvent::L:
        keyinput.l = !pressed;
        break;
    case InputEvent::R:
        keyinput.r = !pressed;
        break;
    default:
        logger.error("Input: handle event %d", static_cast<int>(event));
    }
}

void Input::set_touch(bool pressed) {
    if (pressed) {
        extkeyin &= ~(1 << 6);
    } else {
        extkeyin |= (1 << 6);
    }
}

void Input::set_point(int x, int y) {
    point.x = x;
    point.y = y;
}

} // namespace core