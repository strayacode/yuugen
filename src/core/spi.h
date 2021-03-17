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

    // used to transfer firmware data into main memory
    void DirectBoot();

    void LoadFirmware();


    u16 SPICNT;
    u8 SPIDATA;

    u8 firmware[0x40000];


    Core* core;
};