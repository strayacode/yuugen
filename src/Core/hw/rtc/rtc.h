#pragma once

#include "Common/Types.h"

class RTC {
public:
    void Reset();
    void WriteRTC(u8 data);
    void InterpretWriteCommand(u8 data);

    u8 InterpretReadCommand(u8 data);
    u8 ReadRTC();
    u8 ConvertToBCD(u8 data);

    u8 RTC_REG;
    u8 write_count;
    u8 command;
    u8 status_register1;
    u8 status_register2;
    u8 date_time[7];
};