#pragma once

#include <string.h>
#include <common/log.h>
#include <common/types.h>
#include <functional>

struct Core;

struct DMA {
    DMA(Core* core, int arch);
    void Reset();
    void Transfer(int channel_index);
    void Trigger(u8 mode);

    void WriteDMACNT_L(int channel_index, u16 data);
    void WriteDMACNT_H(int channel_index, u16 data);

    void WriteDMACNT(int channel_index, u32 data);
    auto ReadDMACNT(int channel_index) -> u32;

    auto ReadDMACNT_L(int channel_index) -> u16;
    auto ReadDMACNT_H(int channel_index) -> u16;

    auto ReadLength(int channel_index) -> u32;
    void WriteLength(int channel_index, u32 data);

    Core* core;

    int arch;

    struct DMAChannel {
        // lay out regular and internal registers
        u32 source, internal_source;
        u32 destination, internal_destination;


        // understanding: length and control account for the entire 32 bits of dma cnt
        // with length occupying bits 0..20 and control occupying bits 21..31
        u32 internal_length;
        u32 DMACNT;
    } channel[4];

    std::function<void()> TransferEvent[4];

    u32 DMAFILL[4];  
};