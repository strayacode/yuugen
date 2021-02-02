#include <gba/input.h>

void GBA_Input::handle_keypress(int key, bool key_down) {
    if (key_down) {
        KEYINPUT &= ~(1 << key);
    } else {
        KEYINPUT |= (1 << key);
    }
}