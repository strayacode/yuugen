#pragma once

#include <functional>
#include <string.h>
#include "Common/Log.h"
#include "Common/Types.h"
#include "Core/Scheduler.h"
#include "Core/ARM/MMIO.h"

class System;

class DMA {
public:
    DMA(System& system, int arch);
    
    void reset();
    void build_mmio(MMIO& mmio, Arch arch);

    void Transfer(int channel_index);
    void Trigger(u8 mode);

    void write_dma_length(int channel_index, u16 data);
    void write_dma_control(int channel_index, u16 data);

    u32 ReadLength(int channel_index);
    void WriteLength(int channel_index, u32 data);

    System& system;

    int arch;

    struct DMAChannel {
        // lay out regular and internal registers
        u32 source, internal_source;
        u32 destination, internal_destination;

        // understanding: length and control account for the entire 32 bits of dma cnt
        // with length occupying bits 0..20 and control occupying bits 21..31
        u32 internal_length;
        u32 dmacnt;
    } channel[4];

    u32 DMAFILL[4];

    static constexpr int adjust_lut[2][4] = {
        {2, -2, 0, 2},
        {4, -4, 0, 4},
    };

    EventType transfer_event[4];
};