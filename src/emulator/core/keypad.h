#pragma once
#include <emulator/common/types.h>

class Keypad {
public:
    u16 keyinput = 0x3FF;
    void handle_key(int key, bool key_down);
private:
    
};