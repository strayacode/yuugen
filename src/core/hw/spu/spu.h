#pragma once

#include <common/types.h>
#include <common/log.h>
#include <string.h>
#include <yuugen_common/audio_interface.h>

// remove soon
#include <math.h>

enum ChannelRegisters {
    REG_SOUNDCNT = 0x0,
    REG_SOUNDSAD = 0x4,
    REG_SOUNDTMR = 0x8,
    REG_SOUNDPNT = 0xA,
    REG_SOUNDLEN = 0xC,
};

class SPU;

void AudioCallback(SPU* spu, s16* stream, int len);

class HW;

class SPU {
public:
    SPU(HW* hw);
    ~SPU();
    void Reset();

    auto ReadByte(u32 addr) -> u8;
    auto ReadWord(u32 addr) -> u32;

    void WriteByte(u32 addr, u8 data);
    void WriteHalf(u32 addr, u16 data);
    void WriteWord(u32 addr, u32 data);

    void WriteSOUNDCNT(int channel_index, u32 data);
    void RunChannel(int channel_index);

    // this function will be ran once every 512 cycles
    // and will combine audio data from all channels
    // into a left and right sample to send to the
    // audio buffer
    void RunMixer();

    void SetAudioInterface(AudioInterface& interface);

    struct SPUChannel {
        u32 soundcnt; // sound control
        u32 soundsad; // sound source address
        u16 soundtmr; // timer register
        u16 soundpnt; // loop start register
        u32 soundlen; // sound length register
    } channel[16];

    HW* hw;

    // used by the arm7
    u16 SOUNDCNT;
    u16 SOUNDBIAS;

    u8 SNDCAPCNT[2];
    u32 SNDCAPDAD[2];
    u32 SNDCAPLEN[2];

    AudioInterface* audio_interface;
};