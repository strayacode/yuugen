#pragma once

#include <string>
#include <common/types.h>
#include <common/log.h>
#include <stdio.h>

class GBA_Cartridge {
public:
	~GBA_Cartridge();

	void load_cartridge(std::string rom_path);

	u8 *rom;
private:
};