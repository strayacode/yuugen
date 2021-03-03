#pragma once

#include <string.h>
#include <common/log.h>

struct Core;

struct DMA {
    DMA(Core* core, int arch);

    int arch;


    Core* core;

    struct DMAChannel {
        // lay out regular and internal registers
        u32 source, internal_source;
        u32 destination, internal_destination;


        // understanding: length and control account for the entire 32 bits of dma cnt
        // with length occupying bits 0..20 and control occupying bits 21..31
        u32 internal_length;
        u32 DMACNT;
    } channel[4];

    // bits 0..3 are used to correspond to whether a specific channel 0..3 is enabled
    u8 enabled;

    u32 DMAFILL[4];


    void Reset();

    void Transfer();

    void WriteLength(int channel_index, u32 data);
    void WriteControl(int channel_index, u32 data);
    void WriteDMACNT(int channel_index, u32 data);

};