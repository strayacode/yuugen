#pragma once

#include <util/types.h>

struct Core;

struct SPU {
    SPU(Core* core);
    void Reset();

    // used by the arm7
    u16 SOUNDCNT;

    Core* core;
};