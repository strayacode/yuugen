#pragma once

#include <common/types.h>

struct Core;

struct Interrupt {
    Interrupt(Core* core, int arch);

    void Reset();

    int arch;

    u8 IME;
    u32 IE;
    u32 IF;

    Core* core;
};