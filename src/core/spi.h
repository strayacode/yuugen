#pragma once
#include <util/types.h>
#include <util/log.h>
#include <stdio.h>


struct Core;

struct SPI {
    SPI(Core* core);
    void Reset();
    void WriteSPICNT(u16 data);
    void WriteSPIDATA(u8 data);
    auto ReadSPIDATA() -> u8;

    void Transfer(u8 data);
    void FirmwareTransfer(u8 data);

    // used to transfer firmware data into main memory
    void DirectBoot();

    void LoadFirmware();


    u16 SPICNT;
    u8 SPIDATA;

    // stores the command we need to interpret
    // this is only changed when writecount is 0 (e.g. when the chip gets deselected so thus the command is done)
    u8 command;

    // this is used to keep track of how times spidata has been written to for the current command
    u8 write_count;

    // used in the 3 byte address for command 0x03 so far
    u32 address;

    u8 firmware[0x40000];


    Core* core;
};

// ok so with the 03 command
// first you must get a write to spidata with 03
// then we have 3 more writes to set up the 3 byte address
// after that the 5th write to spidata will cause 