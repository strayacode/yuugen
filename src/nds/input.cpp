#include <nds/input.h>

void NDS_Input::handle_keypress(int key, bool key_down) {
    if (key_down) {
        KEYINPUT &= ~(1 << key);
    } else {
        KEYINPUT |= (1 << key);
    }
}