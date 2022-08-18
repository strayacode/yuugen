#pragma once

#include "Common/Types.h"
#include "Core/ARM/MMIO.h"

class RTC {
public:
    void reset();
    void build_mmio(MMIO& mmio);

    void write_rtc(u8 data);
    void interpret_write_command(u8 data);

    u8 interpret_read_command(u8 data);
    u8 convert_to_bcd(u8 data);

    u8 rtc_register;
    u8 write_count;
    u8 command;
    u8 status_register1;
    u8 status_register2;
    u8 date_time[7];
};