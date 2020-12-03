#pragma once
#include <emulator/core/arm9.h>
#include <emulator/core/arm7.h>
#include <emulator/common/types.h>
#include <emulator/core/memory.h>


class Emulator {
public:
    Emulator();
    void reset();
    ARM9 arm9;
    ARM7 arm7;
    Memory memory;

    bool running;

    


private:
    
};