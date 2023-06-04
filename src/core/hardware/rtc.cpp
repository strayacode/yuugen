#include <ctime>
#include "common/logger.h"
#include "common/bits.h"
#include "core/hardware/rtc.h"

namespace core {

void RTC::reset() {
    rtc.data = 0;
    write_count = 0;
    command = 0;
    status1 = 0;
    status2 = 0;
    date_time.fill(0);
}

void RTC::write_rtc(u8 value) {
    auto old_rtc = rtc;
    rtc.data = value;

    if (!old_rtc.select && rtc.select) {
        if (old_rtc.clock && !rtc.clock) {
            if (write_count < 8) {
                command |= (rtc.data & 0x1) << write_count;
            } else if (rtc.data_io_direction) {
                interpret_write_command(rtc.data);
            } else {
                rtc.data = (rtc.data & ~0x1) | (old_rtc.data & 0x1);
                rtc.data = interpret_read_command(rtc.data);
            }

            write_count++;
        } else if (!rtc.data_io_direction) {
            rtc.data = (rtc.data & ~0x1) | (old_rtc.data & 0x1);
        }
    } else {
        write_count = 0;
        command = 0;
    }
}

u8 RTC::interpret_read_command(u8 value) {
    auto type = (command >> 4) & 0x7;
    switch (type) {
    case 0x0:
        value |= (status1 >> (write_count % 8)) & 0x1;
        break;
    case 0x1:
        // TODO: implement alarm time 1
        break;
    case 0x2: {
        std::time_t time = std::time(nullptr);
        std::tm* current_time = std::localtime(&time);
        current_time->tm_year %= 100;
        current_time->tm_mon++;
        
        date_time[0] = convert_to_bcd(current_time->tm_year);
        date_time[1] = convert_to_bcd(current_time->tm_mon);
        date_time[2] = convert_to_bcd(current_time->tm_mday);
        date_time[3] = convert_to_bcd(current_time->tm_wday);

        if (common::get_bit<1>(status1)) {
            date_time[4] = convert_to_bcd(current_time->tm_hour);
        } else {
            date_time[4] = convert_to_bcd(current_time->tm_hour % 12);
        }

        date_time[5] = convert_to_bcd(current_time->tm_min);
        date_time[6] = convert_to_bcd(current_time->tm_sec);
        value |= (date_time[(write_count / 8) - 1] >> (write_count % 8)) & 0x1;
        break;
    }
    case 0x4:
        value |= (status2 >> (write_count % 8)) & 0x1;
        break;
    case 0x5:
        // TODO: implement alarm time 2
        break;
    case 0x6: {
        std::time_t time = std::time(nullptr);
        std::tm* current_time = std::localtime(&time);
        if (common::get_bit<1>(status1)) {
            date_time[0] = convert_to_bcd(current_time->tm_hour);
        } else {
            date_time[0] = convert_to_bcd(current_time->tm_hour % 12);
        }

        date_time[1] = convert_to_bcd(current_time->tm_min);
        date_time[2] = convert_to_bcd(current_time->tm_sec);
        value |= (date_time[((write_count / 8) % 3) - 1] >> (write_count % 8)) & 0x1;
        break;
    }
    default:
        logger.error("RTC: handle read command %02x", type);
    }

    return value;
}

void RTC::interpret_write_command(u8 value) {
    // TODO: improve the code
    auto type = (command >> 4) & 0x7;
    switch (type) {
    case 0x0:
        status1 = (status1 & ~(1 << (write_count % 8))) | ((value & 0x1) << (write_count % 8));
        break;
    case 0x1:
        // TODO: implement alarm time 1
        break;
    case 0x3:
        // TODO: implement clock adjustment
        break;
    case 0x4:
        status2 = (status2 & ~(1 << (write_count % 8))) | ((value & 0x1) << (write_count % 8));
        break;
    case 0x5:
        // TODO: implement alarm time 2
        break;
    default:
        logger.error("RTC: handle write command %02x", type);
    }
}

u8 RTC::convert_to_bcd(u8 value) {
    return ((value / 10) << 4) | (value % 10);
}

} // namespace core