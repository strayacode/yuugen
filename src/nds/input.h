#pragma once

#include <common/types.h>
#include <common/log.h>

class Input {
public:
	// set to 0x3FF to reflect that all buttons are released
	u16 KEYINPUT = 0x3FF;

	void handle_keypress(int key, bool key_down);

private:
};