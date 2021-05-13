#pragma once
#include <common/types.h>
#include <common/log.h>

struct Core;

struct RTC {
    RTC(Core* core);
    void Reset();

    u8 ReadRTC();
    void WriteRTC(u8 data);

    u8 RTC_REG;

    Core* core;
};