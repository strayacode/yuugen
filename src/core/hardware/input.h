#pragma once

#include "common/types.h"

namespace core {

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

class Input {
public:
    void reset();
    void handle_input(InputEvent event, bool pressed);
    
    u16 read_keyinput() { return keyinput.data & 0x3ff; }
    u16 read_extkeyin() { return extkeyin; }

private:
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