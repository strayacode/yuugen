#pragma once

#include <array>
#include <memory>
#include "Common/Types.h"
#include "AudioCommon/AudioInterface.h"
#include "Core/ARM/MMIO.h"

class System;

// TODO: use synchronous audio (filling a buffer relative to a channels timer)
// (when timer of channel overflows generate a sample) instead of getting all the
// samples when the audio interface needs them

class SPU {
public:
    SPU(System& system);
    ~SPU();

    void reset();
    void build_mmio(MMIO& mmio);

    u8 read_byte(u32 addr);
    u32 read_word(u32 addr);

    void write_byte(u32 addr, u8 data);
    void write_half(u32 addr, u16 data);
    void write_word(u32 addr, u32 data);

    void write_soundcnt(int index, u32 data);
    void run_channel(int index);

    // this function will be ran once every 512 cycles
    // and will combine audio data from all channels
    // into a left and right sample to send to the
    // audio stream
    u32 generate_samples();

    void set_audio_interface(std::shared_ptr<AudioInterface> interface);

    struct SPUChannel {
        u32 soundcnt; // sound control
        u32 soundsad; // sound source address
        u16 soundtmr; // timer register
        u16 soundpnt; // loop start register
        u32 soundlen; // sound length register

        u32 internal_address;
        u16 internal_timer;

        u32 adpcm_header;
        s16 adpcm_value;
        int adpcm_index;

        s16 adpcm_loopstart_value;
        int adpcm_loopstart_index;
        bool adpcm_second_sample;
    } channel[16];

    System& system;

    // used by the arm7
    u16 soundcnt;
    u16 soundbias;

    std::array<u8, 2> sndcapcnt = {};
    std::array<u32, 2> sndcapdad = {};
    std::array<u32, 2> sndcaplen = {};

    std::shared_ptr<AudioInterface> audio_interface;

    static constexpr int index_table[8] = {-1, -1, -1, -1, 2, 4, 6, 8};
    static constexpr int adpcm_table[89] = {
        0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x0010, 0x0011,
        0x0013, 0x0015, 0x0017, 0x0019, 0x001C, 0x001F, 0x0022, 0x0025, 0x0029, 0x002D,
        0x0032, 0x0037, 0x003C, 0x0042, 0x0049, 0x0050, 0x0058, 0x0061, 0x006B, 0x0076,
        0x0082, 0x008F, 0x009D, 0x00AD, 0x00BE, 0x00D1, 0x00E6, 0x00FD, 0x0117, 0x0133,
        0x0151, 0x0173, 0x0198, 0x01C1, 0x01EE, 0x0220, 0x0256, 0x0292, 0x02D4, 0x031C,
        0x036C, 0x03C3, 0x0424, 0x048E, 0x0502, 0x0583, 0x0610, 0x06AB, 0x0756, 0x0812,
        0x08E0, 0x09C3, 0x0ABD, 0x0BD0, 0x0CFF, 0x0E4C, 0x0FBA, 0x114C, 0x1307, 0x14EE,
        0x1706, 0x1954, 0x1BDC, 0x1EA5, 0x21B6, 0x2515, 0x28CA, 0x2CDF, 0x315B, 0x364B,
        0x3BB9, 0x41B2, 0x4844, 0x4F7E, 0x5771, 0x602F, 0x69CE, 0x7462, 0x7FFF
    };
};

void audio_callback(SPU* spu, s16* stream, int len);