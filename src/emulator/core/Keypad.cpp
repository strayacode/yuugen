#include <emulator/common/types.h>
#include <emulator/core/Keypad.h>

void Keypad::handle_key(int key, bool key_down) {
    if (key_down) {
        keyinput &= ~(1 << key);
    }
    
}
