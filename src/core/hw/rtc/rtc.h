#pragma once
#include <common/types.h>
#include <common/log.h>
#include <ctime>

struct Core;

struct RTC {
    RTC(Core* core);
    void Reset();

    auto ReadRTC() -> u8;
    void WriteRTC(u8 data);
    auto InterpretReadCommand(u8 data) -> u8;
    void InterpretWriteCommand(u8 data);
    auto ConvertToBCD(u8 data) -> u8;

    u8 RTC_REG;

    Core* core;

    u8 write_count;
    u8 command;
    u8 status_register1;
    u8 status_register2;

    u8 date_time[7];
};