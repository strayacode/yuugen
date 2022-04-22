#pragma once

#include <vector>
#include "Common/Types.h"

class System;

class SPI {
public:
    SPI(System& system);
    void Reset();
    void WriteSPICNT(u16 data);
    void WriteSPIDATA(u8 data);
    auto ReadSPIDATA() -> u8;

    void Transfer(u8 data);
    void Deselect();
    void FirmwareTransfer(u8 data);
    void TouchscreenTransfer(u8 data);

    // used to transfer firmware data into main memory
    void DirectBoot();

    void LoadFirmware();
    void LoadCalibrationPoints();

    u16 SPICNT;
    u8 SPIDATA;

    // stores the command we need to interpret
    // this is only changed when writecount is 0 (e.g. when the chip gets deselected so thus the command is done)
    u8 command;

    // this is used to keep track of how times spidata has been written to for the current command
    u32 write_count;

    // used in the 3 byte address for command 0x03 so far
    u32 address;

    std::vector<u8> firmware;

    bool write_enable_latch;

    // Write/Program/Erase in Progess
    bool write_in_progress;

    System& system;

    u16 adc_x1;
    u16 adc_x2;
    u16 adc_y1;
    u16 adc_y2;
    u8 scr_x1;
    u8 scr_x2;
    u8 scr_y1;
    u8 scr_y2;

    u16 touch_x = 0x000;
    u16 touch_y = 0xFFF;

    u16 output = 0;
};

// ok so with the 03 command
// first you must get a write to spidata with 03
// then we have 3 more writes to set up the 3 byte address
// after that the 5th write to spidata will cause 