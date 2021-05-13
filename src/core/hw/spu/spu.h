#pragma once

#include <common/types.h>
#include <common/log.h>
#include <string.h>

struct Core;

struct SPU {
    SPU(Core* core);
    void Reset();

    auto ReadByte(u32 addr) -> u8;

    void WriteHalf(u32 addr, u16 data);
    void WriteWord(u32 addr, u32 data);

    struct SPUChannel {
        u32 SOUNDCNT; // sound control
        u32 SOUNDSAD; // sound source address
        u16 SOUNDTMR; // timer register
        u16 SOUNDPNT; // loop start register
        u32 SOUNDLEN; // sound length register

        void WriteSOUNDCNT(u32 data);
    } channel[16];

    Core* core;

    // used by the arm7
    u16 SOUNDCNT;
    u16 SOUNDBIAS;

    u8 SNDCAPCNT[2];
    u32 SNDCAPDAD[2];
    u32 SNDCAPLEN[2];
};