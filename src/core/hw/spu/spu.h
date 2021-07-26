#pragma once

#include <common/types.h>
#include <common/log.h>
#include <string.h>

class HW;

class SPU {
public:
    SPU(HW* hw);
    void Reset();

    auto ReadByte(u32 addr) -> u8;
    auto ReadWord(u32 addr) -> u32;

    void WriteByte(u32 addr, u8 data);
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

    HW* hw;

    // used by the arm7
    u16 SOUNDCNT;
    u16 SOUNDBIAS;

    u8 SNDCAPCNT[2];
    u32 SNDCAPDAD[2];
    u32 SNDCAPLEN[2];
};