#include <ctime>
#include "Common/Log.h"
#include "Core/Hardware/RTC.h"

void RTC::reset() {
    rtc_register = 0;

    write_count = 0;
    command = 0;
    status_register1 = 0;
    status_register2 = 0;

    for (int i = 0; i < 7; i++) {
        date_time[i] = 0;
    }
}

void RTC::build_mmio(MMIO& mmio) {
    mmio.register_mmio<u16>(
        0x04000138,
        mmio.direct_read<u16, u8>(&rtc_register),
        mmio.complex_write<u16>([this](u32, u16 data) {
            write_rtc(data);
        })
    );
}

void RTC::write_rtc(u8 data) {
    // if the cs pinout (bit 2) is set from low to high, then a transfer is started
    if (data & (1 << 2)) {
        // if the sck pinout (bit 1) is set from high to low, then a bit transfer is started
        if ((rtc_register & (1 << 1)) && (!(data & (1 << 1)))) {
            if (write_count < 8) {
                // we still need to set up the command byte for the transfer
                command |= (data & 0x1) << write_count;
            } else {
                if (data & (1 << 4)) {
                    // in this case the data direction is write
                    interpret_write_command(data);
                } else {
                    // in this case the data direction is read
                    // preserve the io bit as we don't want to accidentally overwrite it with the new byte data
                    data &= ~1;

                    // update data when a command is executed
                    data = interpret_read_command(data);
                }
            }
            write_count++;
        } else if (!(data & (1 << 4))) {
            // if no bit transfer happens but we the data direction is read, then preserve the previous io bit
            data = (data & ~1) | (rtc_register & 0x1);
        }
    } else {
        // otherwise the cs pinout was set from high to low, indicating a transfer ended
        // reset write count and command for the next transfer
        write_count = 0;
        command = 0;
    }
    rtc_register = data;
}

u8 RTC::interpret_read_command(u8 data) {
    // extract the specific command from the command byte
    u8 command_type = (command >> 4) & 0x7;
    switch (command_type) {
    case 0:
        // read status register 1
        // get a specific bit dependent on the write count
        data |= (status_register1 >> (write_count % 8)) & 0x1;
        break;
    case 1:
        // alarm time 1
        // do nothing for now lol
        break;
    case 2: {
        // date / time
        // get the current time
        std::time_t time = std::time(nullptr);
        std::tm* current_time = std::localtime(&time);
        current_time->tm_year %= 100;
        current_time->tm_mon++;
        
        // update date time
        date_time[0] = convert_to_bcd(current_time->tm_year);
        date_time[1] = convert_to_bcd(current_time->tm_mon);
        date_time[2] = convert_to_bcd(current_time->tm_mday);
        date_time[3] = convert_to_bcd(current_time->tm_wday);

        if (status_register1 & (1 << 1)) {
            // 24 hour mode
            date_time[4] = convert_to_bcd(current_time->tm_hour);
        } else {
            // 12 hour mode
            date_time[4] = convert_to_bcd(current_time->tm_hour % 12);
        }

        date_time[5] = convert_to_bcd(current_time->tm_min);
        date_time[6] = convert_to_bcd(current_time->tm_sec);
        
        // update bit 0
        // since each date time value is a byte, we can only use a different parameter byte every 8 write counts
        data |= (date_time[(write_count / 8) - 1] >> (write_count % 8)) & 0x1;
        break;
    }
    case 4:
        // read status register 2
        // get a specific bit dependent on the write count
        data |= (status_register2 >> (write_count % 8)) & 0x1;
        break;
    case 5:
        // alarm time 2
        // do nothing for now lol
        break;
    case 6: {
        // time
        std::time_t time = std::time(nullptr);
        std::tm* current_time = std::localtime(&time);
        if (status_register1 & (1 << 1)) {
            // 24 hour mode
            date_time[0] = convert_to_bcd(current_time->tm_hour);
        } else {
            // 12 hour mode
            date_time[0] = convert_to_bcd(current_time->tm_hour % 12);
        }

        date_time[1] = convert_to_bcd(current_time->tm_min);
        date_time[2] = convert_to_bcd(current_time->tm_sec);

         // update bit 0
        // since each date time value is a byte, we can only use a different parameter byte every 8 write counts
        data |= (date_time[((write_count / 8) % 3) - 1] >> (write_count % 8)) & 0x1;
        break;
    }
    default:
        log_fatal("[RTC] Handle read command type %02x", command_type);
    }

    return data;
}

void RTC::interpret_write_command(u8 data) {
    // extract the specific command from the command byte
    u8 command_type = (command >> 4) & 0x7;
    switch (command_type) {
    case 0:
        status_register1 = (status_register1 & ~(1 << (write_count % 8))) | ((data & 0x1) << (write_count % 8));
        break;
    case 1:
        // alarm time 1
        // do nothing for now lol
        break;
    case 3:
        // clock adjustment
        break;
    case 4:
        status_register2 = (status_register2 & ~(1 << (write_count % 8))) | ((data & 0x1) << (write_count % 8));
        break;
    case 5:
        // alarm time 2
        // do nothing for now lol
        break;
    default:
        log_fatal("[RTC] Handle write command type %02x", command_type);
    }
}

u8 RTC::convert_to_bcd(u8 data) {
    // the tens digit will be stored in the top 4 bits and the ones digit in the bottom 4 bits
    u8 result = ((data / 10) << 4) | ((data % 10));

    return result;
}