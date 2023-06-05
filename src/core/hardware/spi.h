#pragma once

#include "common/types.h"
#include "common/regular_file.h"

namespace core {

class System;

class SPI {
public:
    SPI(System& system);

    void reset();
    void direct_boot();

    u16 read_spicnt() { return spicnt.data; }
    u8 read_spidata() { return spidata; }

    void write_spicnt(u16 value, u32 mask);
    void write_spidata(u8 value);

private:
    void transfer(u8 value);
    void firmware_transfer(u8 value);
    void touchscreen_transfer(u8 value);

    enum Device : int {
        Powerman = 0,
        Firmware = 1,
        Touchscreen = 2,
        Reserved = 3,
    };

    union SPICNT {
        struct {
            u16 baudrate : 2;
            u16 : 5;
            bool busy : 1;
            Device device : 2;
            bool transfer_halfwords : 1;
            bool chipselect_hold : 1;
            u16 : 2;
            bool irq : 1;
            bool enable : 1;
        };

        u16 data;
    };

    SPICNT spicnt;
    u8 spidata;
    int write_count;
    bool write_enable_latch;
    bool write_in_progress;
    u8 command;
    u32 address;
    common::RegularFile firmware;
    System& system;
};

} // namespace core