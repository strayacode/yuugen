#pragma once

#include "common/callback.h"

namespace common {

enum class InputEvent : int {
    A,
    B,
    Start,
    Select,
    Left,
    Right,
    Up,
    Down,
    L,
    R,
};

class InputDevice {
public:
    using InputCallback = common::Callback<void(InputEvent, bool)>;

    void set_input_callback(InputCallback callback) {
        this->callback = callback;
    }

    void handle(InputEvent event, bool pressed) {
        callback(event, pressed);
    }

private:
    InputCallback callback;
};

} // namespace common