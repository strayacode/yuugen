#pragma once

#include <string>

class EmuCore {
public:
	EmuCore() {};
	~EmuCore() {};

	// these class methods are common between the GBA and NDS class which allows us to use EmuCore as the class type for both
	virtual void direct_boot(std::string rom_path) = 0;
	virtual void run_frame() = 0;
	virtual void reset() = 0;
private:
};