#pragma once

#include <util/types.h>

struct Core;



struct SPU {
    SPU(Core* core);
    struct SPU_CHANNEL {
        u32 SOUNDCNT; // sound control
        u32 SOUNDSAD; // sound source address
        u16 SOUNDTMR; // timer register
        u16 SOUNDPNT; // loop start register
        u32 SOUNDLEN; // sound length register

        void WriteSOUNDCNT(u32 data);
    } channel[16];

    void Reset();

    void WriteSoundChannel(u32 addr, u32 data);

    // used by the arm7
    u16 SOUNDCNT;

    Core* core;
};