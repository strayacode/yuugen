#pragma once
#include <gba/arm.h>
#include <common/emu_core.h>
#include <string>

class GBA : virtual public EmuCore {
public:
	void direct_boot(std::string rom_path);
private:
	ARM arm7;
};