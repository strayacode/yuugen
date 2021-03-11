#pragma once
#include <common/types.h>


struct Core;

struct SPI {
    SPI(Core* core);
    void Reset();
    void WriteSPICNT(u16 data);
    void WriteSPIDATA(u8 data);


    u16 SPICNT;
    u8 SPIDATA;


    Core* core;
};