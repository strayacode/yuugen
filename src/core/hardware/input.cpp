#include "common/logger.h"
#include "core/hardware/input.h"

namespace core {

void Input::reset() {
    keyinput.data = 0x3ff;
    extkeyin = 0x7f;
}

void Input::set_input_device(common::InputDevice& input_device) {
    input_device.set_input_callback([this](common::InputEvent event, bool pressed) {
        input_callback(event, pressed);
    });
}

u16 Input::read_keyinput() {
    return keyinput.data & 0x3ff;
}

u16 Input::read_extkeyin() {
    return extkeyin;
}

void Input::input_callback(common::InputEvent event, bool pressed) {
    switch (event) {
    case common::InputEvent::A:
        keyinput.a = !pressed;
        break;
    case common::InputEvent::B:
        keyinput.b = !pressed;
        break;
    case common::InputEvent::Start:
        keyinput.start = !pressed;
        break;
    case common::InputEvent::Select:
        keyinput.select = !pressed;
        break;
    case common::InputEvent::Left:
        keyinput.left = !pressed;
        break;
    case common::InputEvent::Right:
        keyinput.right = !pressed;
        break;
    case common::InputEvent::Up:
        keyinput.up = !pressed;
        break;
    case common::InputEvent::Down:
        keyinput.down = !pressed;
        break;
    case common::InputEvent::L:
        keyinput.l = !pressed;
        break;
    case common::InputEvent::R:
        keyinput.r = !pressed;
        break;
    default:
        logger.error("Input: handle event %d", static_cast<int>(event));
    }
}

} // namespace core