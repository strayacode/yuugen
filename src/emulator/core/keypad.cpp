#include <emulator/common/types.h>
#include <emulator/core/keypad.h>

void Keypad::handle_key(int key, bool key_down) {
    if (key_down) {
        keyinput &= ~(1 << key);
    }
    
}
