#pragma once
#include <util/types.h>
#include <util/log.h>

struct Core;

struct RTC {
    RTC(Core* core);
    void Reset();

    u8 RTC_REG;

    Core* core;

};