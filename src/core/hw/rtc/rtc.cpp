#include <core/hw/rtc/rtc.h>
#include <core/hw/hw.h>

RTC::RTC(HW* hw) : hw(hw) {

}

void RTC::Reset() {
    RTC_REG = 0;

    write_count = 0;
    command = 0;
    status_register1 = 0;
    status_register2 = 0;

    for (int i = 0; i < 7; i++) {
        date_time[i] = 0;
    }
}

auto RTC::ReadRTC() -> u8 {
    return RTC_REG;
}

void RTC::WriteRTC(u8 data) {
    // if the cs pinout (bit 2) is set from low to high, then a transfer is started
    if (data & (1 << 2)) {
        // if the sck pinout (bit 1) is set from high to low, then a bit transfer is started
        if ((RTC_REG & (1 << 1)) && (!(data & (1 << 1)))) {
            if (write_count < 8) {
                // we still need to set up the command byte for the transfer
                command |= (data & 0x1) << write_count;
            } else {
                if (data & (1 << 4)) {
                    // in this case the data direction is write
                    InterpretWriteCommand(data);
                } else {
                    // in this case the data direction is read
                    // preserve the io bit as we don't want to accidentally overwrite it with the new byte data
                    data &= ~1;

                    // update data when a command is executed
                    data = InterpretReadCommand(data);
                }
            }
            write_count++;
        } else if (!(data & (1 << 4))) {
            // if no bit transfer happens but we the data direction is read, then preserve the previous io bit
            data = (data & ~1) | (RTC_REG & 0x1);
        }
    } else {
        // otherwise the cs pinout was set from high to low, indicating a transfer ended
        // reset write count and command for the next transfer
        write_count = 0;
        command = 0;
    }
    RTC_REG = data;
}

auto RTC::InterpretReadCommand(u8 data) -> u8 {
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
        date_time[0] = ConvertToBCD(current_time->tm_year);
        date_time[1] = ConvertToBCD(current_time->tm_mon);
        date_time[2] = ConvertToBCD(current_time->tm_mday);
        date_time[3] = ConvertToBCD(current_time->tm_wday);

        if (status_register1 & (1 << 1)) {
            // 24 hour mode
            date_time[4] = ConvertToBCD(current_time->tm_hour);
        } else {
            // 12 hour mode
            date_time[4] = ConvertToBCD(current_time->tm_hour % 12);
        }

        date_time[5] = ConvertToBCD(current_time->tm_min);
        date_time[6] = ConvertToBCD(current_time->tm_sec);
        
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
            date_time[0] = ConvertToBCD(current_time->tm_hour);
        } else {
            // 12 hour mode
            date_time[0] = ConvertToBCD(current_time->tm_hour % 12);
        }

        date_time[1] = ConvertToBCD(current_time->tm_min);
        date_time[2] = ConvertToBCD(current_time->tm_sec);

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

void RTC::InterpretWriteCommand(u8 data) {
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

auto RTC::ConvertToBCD(u8 data) -> u8 {
    // the tens digit will be stored in the top 4 bits and the ones digit in the bottom 4 bits
    u8 result = ((data / 10) << 4) | ((data % 10));

    return result;
}