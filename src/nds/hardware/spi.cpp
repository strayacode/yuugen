#include "common/logger.h"
#include "common/bits.h"
#include "common/memory.h"
#include "nds/hardware/spi.h"
#include "nds/system.h"

namespace nds {

SPI::SPI(System& system) : system(system) {}

void SPI::reset() {
    spicnt.data = 0;
    spidata = 0;
    write_count = 0;
    write_enable_latch = false;
    write_in_progress = false;
    command = 0;
    address = 0;
    output = 0;

    firmware.load("../firmware/firmware.bin");
    load_calibration_points();
}

void SPI::direct_boot() {
    for (u32 i = 0; i < 0x70; i++) {
        system.arm9.get_memory().write<u8, arm::Bus::System>(0x027ffc80 + i, *firmware.get_pointer(0x3ff00 + i));
    }
}

void SPI::write_spicnt(u16 value, u32 mask) {
    mask &= 0xcf03;
    spicnt.data = (spicnt.data & ~mask) | (value & mask);
}

void SPI::write_spidata(u8 value) {
    LOG_WARN("spidata write %02x", value);
    if (spicnt.enable) {
        transfer(value);
    } else {
        spidata = 0;
    }
}

void SPI::transfer(u8 value) {
    if (write_count == 0) {
        command = value;
        address = 0;
        spidata = 0;
    } else {
        switch (static_cast<Device>(spicnt.device)) {
        case Device::Powerman:
            // TODO: figure what to actually do here
            spidata = 0;
            break;
        case Device::Firmware:
            firmware_transfer(value);
            break;
        case Device::Touchscreen:
            touchscreen_transfer(value);
            break;
        case Device::Reserved:
            LOG_ERROR("handle reserved transfer");
            break;
        }
    }

    if (spicnt.chipselect_hold) {
        write_count++;
    } else {
        write_count = 0;
    }

    if (spicnt.irq) {
        system.arm7.get_irq().raise(IRQ::Source::SPI);
    }
}

void SPI::firmware_transfer(u8 value) {
    if (spicnt.transfer_halfwords) {
        LOG_ERROR("handle bugged 16-bit transfer");
    }

    switch (command) {
    case 0x03:
        if (write_count < 4) {
            address |= value << ((3 - write_count) * 8);
        } else {
            if (address >= 0x40000) {
                LOG_ERROR("illegal firmware address");
            }

            spidata = *firmware.get_pointer(address);
            address += spicnt.transfer_halfwords ? 2 : 1;
        }

        break;
    case 0x05:
        spidata = write_in_progress | (write_enable_latch << 1);
        break;
    default:
        LOG_ERROR("unimplemented firmware command %02x", command);
    }
}

void SPI::touchscreen_transfer(u8 value) {
    u8 upper = output >> 8;
    output <<= 8;

    if (common::get_bit<7>(value)) {
        u8 channel = common::get_field<4, 3>(value);
        u16 touch_x = 0;
        u16 touch_y = 0xfff;

        if (system.input.touch_down()) {
            touch_x = (system.input.point.x - scr_x1 + 1) * (adc_x2 - adc_x1) / (scr_x2 - scr_x1) + adc_x1;
            touch_y = (system.input.point.y - scr_y1 + 1) * (adc_y2 - adc_y1) / (scr_y2 - scr_y1) + adc_y1;
            
            switch (channel) {
            case 1:
                output = touch_y << 3;
                break;
            case 5:
                output = touch_x << 3;
                break;
            default:
                spidata = 0;
                break;
            }
        }
    }

    spidata = upper;
}

void SPI::load_calibration_points() {
    u32 user_settings_offset = common::read<u16>(firmware.get_pointer(0x20)) * 8;
    
    adc_x1 = common::read<u16>(firmware.get_pointer(user_settings_offset + 0x58));
    adc_y1 = common::read<u16>(firmware.get_pointer(user_settings_offset + 0x5a));
    scr_x1 = common::read<u8>(firmware.get_pointer(user_settings_offset + 0x5c));
    scr_y1 = common::read<u8>(firmware.get_pointer(user_settings_offset + 0x5d));
    adc_x2 = common::read<u16>(firmware.get_pointer(user_settings_offset + 0x5e));
    adc_y2 = common::read<u16>(firmware.get_pointer(user_settings_offset + 0x60));
    scr_x2 = common::read<u8>(firmware.get_pointer(user_settings_offset + 0x62));
    scr_y2 = common::read<u8>(firmware.get_pointer(user_settings_offset + 0x63));
    LOG_DEBUG("touchscreen calibration points loaded successfully");
}

} // namespace nds