#pragma once
#include <nds/core/arm9.h>
#include <nds/core/arm7.h>
#include <nds/common/types.h>
#include <nds/core/memory.h>
#include <nds/core/cp15.h>


class NDS {
public:
    NDS();
    void reset();
    void run();
    ARM9 arm9;
    ARM7 arm7;
    Memory memory;
    CP15 cp15;

    bool running;

    


private:
    
};