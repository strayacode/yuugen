#pragma once
#include <emulator/core/arm.h>
#include <emulator/common/types.h>
#include <emulator/core/memory.h>
#include <emulator/core/cartridge.h>
#include <emulator/core/cp15.h>
#include <string>

class Emulator {
public:
    Emulator();
    void reset();
    void run(std::string rom_path);
    ARM arm9;
    ARM arm7;
    Memory memory;
    Cartridge cartridge;
    CP15 cp15;

    bool running;

    


private:
    
};