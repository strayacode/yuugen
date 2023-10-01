#pragma once

#include "common/types.h"
#include "common/regular_file.h"

namespace nds {

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
    void load_calibration_points();

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
            u16 device : 2;
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

    u16 adc_x1;
    u16 adc_x2;
    u16 adc_y1;
    u16 adc_y2;
    u8 scr_x1;
    u8 scr_x2;
    u8 scr_y1;
    u8 scr_y2;
    u16 output;

    common::RegularFile firmware;
    System& system;
};

} // namespace nds