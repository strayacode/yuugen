#include <core/hw/spi/spi.h>
#include <core/core.h>


SPI::SPI(Core* core) : core(core) {
    
}

void SPI::Reset() {
    firmware.clear();

    LoadFirmware();
    LoadCalibrationPoints();

    SPICNT = 0;
    SPIDATA = 0;

    write_enable_latch = false;
    write_in_progress = false;
}

void SPI::WriteSPICNT(u16 data) {
    SPICNT = (SPICNT & ~0xCF03) | (data & 0xCF03);
}

auto SPI::ReadSPIDATA() -> u8 {
    return SPIDATA;
}

void SPI::WriteSPIDATA(u8 data) {
    // check if spi is enabled
    if (SPICNT & (1 << 15)) {
        Transfer(data);
    } else {
        SPIDATA = 0;
    }
}

void SPI::DirectBoot() {
    // write user settings 1 (0x70 in length) to address 0x027FFC80 in main memory
    for (u32 i = 0; i < 0x70; i++) {
        core->memory.ARM9Write<u8>(0x027FFC80 + i, firmware[0x3FF00 + i]);
    }
}

void SPI::LoadFirmware() {
    std::ifstream file("../firmware/firmware.bin", std::ios::binary);

    if (!file) {
        log_fatal("[SPI] Firmware could not be found");
    }

    file.unsetf(std::ios::skipws);

    std::streampos size;

    file.seekg(0, std::ios::beg);

    firmware.reserve(0x40000);

    firmware.insert(firmware.begin(), std::istream_iterator<u8>(file), std::istream_iterator<u8>());

    file.close();

    log_debug("[SPI] Firmware loaded successfully!");
}

void SPI::LoadCalibrationPoints() {
    // get the user settings address from address 0x20 in the firmware header
    u32 user_settings_offset = 0;
    memcpy(&user_settings_offset, &firmware[0x20], 2);

    // in the firmware header, the offset is actually stored as the address divided by 8, so we must reverse the division to get the actual address
    user_settings_offset *= 8;

    adc_x1 = 0;
    adc_x2 = 0;
    adc_y1 = 0;
    adc_y2 = 0;
    scr_x1 = 0;
    scr_x2 = 0;
    scr_y1 = 0;
    scr_y2 = 0;
    memcpy(&adc_x1, &firmware[user_settings_offset + 0x58], 2);
    memcpy(&adc_y1, &firmware[user_settings_offset + 0x5A], 2);
    memcpy(&scr_x1, &firmware[user_settings_offset + 0x5C], 1);
    memcpy(&scr_y1, &firmware[user_settings_offset + 0x5D], 1);
    memcpy(&adc_x2, &firmware[user_settings_offset + 0x5E], 2);
    memcpy(&adc_y2, &firmware[user_settings_offset + 0x60], 2);
    memcpy(&scr_x2, &firmware[user_settings_offset + 0x62], 1);
    memcpy(&scr_y2, &firmware[user_settings_offset + 0x63], 1);

    log_debug("[SPI] Touchscreen calibration points loaded successfully");
}

void SPI::Transfer(u8 data) {
    if (write_count == 0) {
        // set the new command to the spidata
        command = data;
        // reset the address used
        address = 0;
        // reset the spidata so that when we read a byte from firmware into it
        // we just have a fresh spidata
        SPIDATA = 0;
    } else {
        // if the command has been set on the previous spidata write then we can now interpret the command
        // check device select to see which device is used for transfer
        u8 device_select = (SPICNT >> 8) & 0x3;
        switch (device_select) {
        case 0:
            // just set spidata to 0
            SPIDATA = 0;
            break;
        case 1:
            FirmwareTransfer(data);
            break;
        case 2:
            TouchscreenTransfer(data);
            break;
        default:
            log_fatal("device %d is not implemented yet for spi transfers", device_select);
        }
    }

    // keep device selected or deselect it depending on bit 11 of spicnt
    if (SPICNT & (1 << 11)) {
        // continue with the command
        write_count++;
    } else {
        // interpret a new command
        write_count = 0;
    }

    // if enabled trigger a transfer finished irq
    if (SPICNT & (1 << 14)) {
        core->arm7.SendInterrupt(23);
    }
}

void SPI::FirmwareTransfer(u8 data) {
    if (SPICNT & (1 << 10)) {
        log_fatal("[SPI] Implement support for bugged 16-bit transfer size");
    }

    // NOTE: the write enable latch is reset on WRDI/PW/PP/PE/SE instructions
    // and is set on the WREN instruction

    // interpret a command
    switch (command) {
    case 0x03:
        // what we do will depend on the write count
        // on write_count < 4, we will set up the 3 byte address
        // on write_count >= 4, we will finally read a byte from the firmware
        // in setting up the 3 byte address the most significant byte is done first
        if (write_count < 4) {
            address |= (data << ((3 - write_count) * 8));
        } else {
            // read endless stream of bytes (keep incrementing the address unless chip is deselected)
            // read data from firmware
            if (address >= 0x40000) {
                log_fatal("undefined firmware address");
            }
            SPIDATA = firmware[address];

            // increment the address after reading a byte into SPIDATA
            address++;
        }
        break;
    case 0x05:
        // read status register
        SPIDATA = (write_in_progress ? 1 : 0) | (write_enable_latch ? (1 << 1) : 0);
        break;
    default:
        log_fatal("implement support for firmware command %02x", command);
    }
}

void SPI::TouchscreenTransfer(u8 data) {
    // each time we do a touchscreen transfer, we want to transfer the msb first
    u8 value = output >> 8;

    // shift the lsb to be the msb now
    output <<= 8;

    // bit 7 signifies the start bit and tells us whether the control byte (data in this case) can be accessed
    if (data & (1 << 7)) {
        // determine the channel to access
        u8 channel = (data >> 4) & 0x7;

        // only return true touchscreen points if the touchscreen is currently pressed
        if (core->input.TouchDown()) {
            // the formula in gbatek is used to convert touchscreen values to screen / pixel values
            // with some rearrangement we get the formula to convert screen / pixel values to touchscreen values
            touch_x = (core->input.point.x - scr_x1 + 1) * (adc_x2 - adc_x1) / (scr_x2 - scr_x1) + adc_x1;
            touch_y = (core->input.point.y - scr_y1 + 1) * (adc_y2 - adc_y1) / (scr_y2 - scr_y1) + adc_y1;

            switch (channel) {
            case 1:
                output = touch_y << 3;
                break;
            case 5:
                output = touch_x << 3;
                break;
            default:
                SPIDATA = 0;
                break;
            }
        }
    }

    SPIDATA = value;
}