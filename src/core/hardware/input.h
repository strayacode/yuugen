#pragma once

#include "common/types.h"
#include "common/input_device.h"

namespace core {

class Input {
public:
    void reset();
    void set_input_device(common::InputDevice& input_device);

    u16 read_keyinput();
    u16 read_extkeyin();

private:
    void input_callback(common::InputEvent event, bool pressed);

    union KEYINPUT {
        struct {
            bool a : 1;
            bool b : 1;
            bool select : 1;
            bool start : 1;
            bool right : 1;
            bool left : 1;
            bool up : 1;
            bool down : 1;
            bool r : 1;
            bool l : 1;
            u16 : 6;
        };

        u16 data;
    };

    KEYINPUT keyinput;
    u16 extkeyin;
};

} // namespace core