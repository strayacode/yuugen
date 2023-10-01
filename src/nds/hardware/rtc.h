#pragma once

#include <array>
#include "common/types.h"

namespace nds {

class RTC {
public:
    void reset();

    u8 read_rtc() { return rtc.data; }
    void write_rtc(u8 value);

private:
    u8 interpret_read_command(u8 value);
    void interpret_write_command(u8 value);

    u8 convert_to_bcd(u8 value);

    union Register {
        struct {
            bool data_io : 1;
            bool clock : 1;
            bool select : 1;
            u8 : 1;
            bool data_io_direction : 1;
            bool clock_direction : 1;
            bool select_direction : 1;
            u8 : 1;
        };

        u8 data;
    };

    Register rtc;
    u8 write_count;
    u8 command;
    u8 status1;
    u8 status2;
    std::array<u8, 7> date_time;
};

} // namespace nds