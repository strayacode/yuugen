#pragma once
#include <emulator/core/arm.h>
#include <emulator/common/types.h>
#include <emulator/core/memory.h>


class Emulator {
public:
    Emulator();
    ARM9 arm9;
    ARM7 arm7;
    Memory memory;

    

    


private:
    
};