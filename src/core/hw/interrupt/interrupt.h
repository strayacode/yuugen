#pragma once

#include <common/types.h>

class HW;

class Interrupt {
public:
    Interrupt(HW* hw, int arch);

    void Reset();

    u8 IME;
    u32 IE;
    u32 IF;

    HW* hw;

    int arch;
};