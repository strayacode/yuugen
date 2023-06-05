#include "common/logger.h"
#include "core/hardware/spi.h"
#include "core/system.h"

namespace core {

SPI::SPI(System& system) : system(system) {}

void SPI::reset() {
    spicnt.data = 0;
    spidata = 0;
    write_count = 0;
    write_enable_latch = false;
    write_in_progress = false;
    command = 0;
    address = 0;

    firmware.load("../firmware/firmware.bin");
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
        switch (spicnt.device) {
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
            logger.error("SPI: handle reserved transfer");
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
        logger.error("SPI: handle bugged 16-bit transfer");
    }

    switch (command) {
    case 0x03:
        if (write_count < 4) {
            address |= value << ((3 - write_count) * 8);
        } else {
            if (address >= 0x40000) {
                logger.error("SPI: illegal firmware address");
            }

            spidata = *firmware.get_pointer(address);
            address += spicnt.transfer_halfwords ? 2 : 1;
        }

        break;
    case 0x05:
        spidata = write_in_progress | (write_enable_latch << 1);
        break;
    default:
        logger.error("SPI: unimplemented firmware command %02x", command);
    }
}

void SPI::touchscreen_transfer(u8 value) {
    logger.error("SPI: handle touchscreen transfer");
}

} // namespace core