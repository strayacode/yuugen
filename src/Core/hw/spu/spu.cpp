#include <algorithm>
#include "Common/Log.h"
#include "Common/Settings.h"
#include "Core/hw/spu/spu.h"
#include "Core/core.h"

// sound notes:
// for pcm audio data,
// pcm8 data NN will have
// the same volume as pcm16 data
// NN00

SPU::SPU(System& system) : system(system) {
    
}

SPU::~SPU() {
    if (audio_interface != nullptr) {
        audio_interface->Close();
    }
}

void SPU::Reset() {
    soundcnt = 0;
    soundbias = 0;
    for (int i = 0; i < 16; i++) {
        memset(&channel[i], 0, sizeof(SPUChannel));
    }

    SNDCAPCNT[0] = SNDCAPCNT[1] = 0;
    SNDCAPDAD[0] = SNDCAPDAD[1] = 0;
    SNDCAPLEN[0] = SNDCAPLEN[1] = 0;
}

auto SPU::ReadByte(u32 addr) -> u8 {
    u8 channel_index = (addr >> 4) & 0xF;

    switch (addr & 0xF) {
    case REG_SOUNDCNT + 2:
        // get 3rd byte of SOUNDCNT for that channel
        return ((channel[channel_index].soundcnt >> 16) & 0xFF);
    case REG_SOUNDCNT + 3:
        // get upper byte of SOUNDCNT for that channel
        return ((channel[channel_index].soundcnt >> 24) & 0xFF);
    default:
        log_fatal("[SPU] Unhandled address %08x", addr & 0xF);
    }
}

auto SPU::ReadWord(u32 addr) -> u32 {
    u8 channel_index = (addr >> 4) & 0xF;

    switch (addr & 0xF) {
    case REG_SOUNDCNT:
        return channel[channel_index].soundcnt;
    default:
        log_fatal("[SPU] Unhandled address %08x", addr & 0xF);
    }
}

void SPU::WriteByte(u32 addr, u8 data) {
    u8 channel_index = (addr >> 4) & 0xF;
    // check which data in the channel should be changed
    switch (addr & 0xF) {
    case REG_SOUNDCNT:
        // write to 1st byte of SOUNDCNT
        channel[channel_index].soundcnt = (channel[channel_index].soundcnt & ~0x000000FF) | data;
        break;
    case REG_SOUNDCNT + 2:
        // write to 3rd byte of SOUNDCNT
        channel[channel_index].soundcnt = (channel[channel_index].soundcnt & ~0x00FF0000) | (data << 16);
        break;
    case REG_SOUNDCNT + 3:
        // write to 4th byte of SOUNDCNT
        WriteSOUNDCNT(channel_index, (channel[channel_index].soundcnt & ~0xFF000000) | (data << 24));
        break;
    default:
        log_fatal("part of sound channel %02x not handled yet", addr & 0xF);
    }
}

void SPU::WriteHalf(u32 addr, u16 data) {
    u8 channel_index = (addr >> 4) & 0xF;
    // check which data in the channel should be changed
    switch (addr & 0xF) {
    case REG_SOUNDCNT:
        // write to lower halfword of SOUNDCNT
        channel[channel_index].soundcnt = (channel[channel_index].soundcnt & ~0xFFFF) | (data & 0xFFFF);
        break;
    case REG_SOUNDTMR:
        // write to lower halfword of SOUNDTMR
        channel[channel_index].soundtmr = data;
        break;
    case REG_SOUNDPNT:
        // write to SOUNDPNT
        channel[channel_index].soundpnt = data;
        break;
    case REG_SOUNDLEN:
        // write to SOUNDLEN
        channel[channel_index].soundlen = data;
        break;
    default:
        log_fatal("part of sound channel %02x not handled yet", addr & 0xF);
    }
}

void SPU::WriteWord(u32 addr, u32 data) {
    // get the channel index
    u8 channel_index = (addr >> 4) & 0xF;
    // check which data in the channel should be changed
    switch (addr & 0xF) {
    case REG_SOUNDCNT:
        WriteSOUNDCNT(channel_index, data);
        break;
    case REG_SOUNDSAD:
        channel[channel_index].soundsad = data & 0x07FFFFFC;
        break;
    case REG_SOUNDTMR:
        channel[channel_index].soundtmr = data;
        break;
    case REG_SOUNDLEN:
        channel[channel_index].soundlen = data & 0x3FFFFF;
        break;
    default:
        log_fatal("part of sound channel %02x not handled yet", addr & 0xF);
    }
}

void SPU::WriteSOUNDCNT(int channel_index, u32 data) {
    // start the channel
    if (!(channel[channel_index].soundcnt >> 31) && (data >> 31)) {
        RunChannel(channel_index);
    }

    channel[channel_index].soundcnt = data;
}

void SPU::RunChannel(int channel_index) {
    // reload internal registers
    channel[channel_index].internal_address = channel[channel_index].soundsad;
    channel[channel_index].internal_timer = channel[channel_index].soundtmr;

    u8 format = (channel[channel_index].soundcnt >> 29) & 0x3;

    switch (format) {
    case 0x2:
        // read in the adpcm header
        channel[channel_index].adpcm_header = system.arm7_memory.FastRead<u32>(channel[channel_index].internal_address);
        channel[channel_index].adpcm_value = (s16)channel[channel_index].adpcm_header;
        channel[channel_index].adpcm_index = std::min((channel[channel_index].adpcm_header >> 16) & 0x7F, 88U);
        channel[channel_index].internal_address += 4;
        break;
    case 0x3:
        // log_warn("handle psg/noise");
        break;
    }
}

u32 SPU::GenerateSamples() {
    s64 sample_left = 0;
    s64 sample_right = 0;

    for (int i = 0; i < 16; i++) {
        // don't mix audio from channels
        // that are disabled
        if (!(channel[i].soundcnt >> 31)) {
            continue;
        }

        s64 data = 0;
        u8 data_size = 0;
        u8 format = (channel[i].soundcnt >> 29) & 0x3;

        switch (format) {
        case 0x1:
            data = (s16)system.arm7_memory.FastRead<u16>(channel[i].internal_address);
            data_size = 2;
            break;
        case 0x2:
            data = channel[i].adpcm_value;
            break;
        default:
            // log_warn("[SPU] Handle format %d", format);
            break;
        }

        // 512 cycles are used up before a sample can be generated
        // TODO: make this correctly timed against the system clock
        for (int j = 0; j < 512; j++) {
            channel[i].internal_timer++;
        
            if (channel[i].internal_timer == 0) {
                // overflow occured
                channel[i].internal_timer = channel[i].soundtmr;
                channel[i].internal_address += data_size;

                if (format == 2) {
                    // decode adpcm data
                    u8 adpcm_data = system.arm7_memory.FastRead<u8>(channel[i].internal_address);

                    // each sample is 4-bit
                    if (channel[i].adpcm_second_sample) {
                        adpcm_data = (adpcm_data >> 4) & 0xF;
                    } else {
                        adpcm_data &= 0xF;
                    }

                    int diff = adpcm_table[channel[i].adpcm_index] / 8;

                    if (adpcm_data & 0x1) {
                        diff += adpcm_table[channel[i].adpcm_index] / 4;
                    }

                    if (adpcm_data & (1 << 1)) {
                        diff += adpcm_table[channel[i].adpcm_index] / 2;
                    }

                    if (adpcm_data & (1 << 2)) {
                        diff += adpcm_table[channel[i].adpcm_index];
                    }

                    if (adpcm_data & (1 << 3)) {
                        channel[i].adpcm_value = std::max(channel[i].adpcm_value - diff, -0x7FFF);
                    } else {
                        channel[i].adpcm_value = std::min(channel[i].adpcm_value + diff, 0x7FFF);
                    }

                    channel[i].adpcm_index = std::clamp(channel[i].adpcm_index + index_table[adpcm_data & 0x7], 0, 88);

                    // toggle between the first and second byte
                    channel[i].adpcm_second_sample = !channel[i].adpcm_second_sample;

                    // go to next byte once a second sample has been generated
                    if (!channel[i].adpcm_second_sample) {
                        channel[i].internal_address++;
                    }

                    // save the value and index if we are at the loopstart location
                    // and are on the first byte
                    if ((channel[i].internal_address == channel[i].soundsad + (channel[i].soundpnt * 4)) && !channel[i].adpcm_second_sample) {
                        channel[i].adpcm_loopstart_value = channel[i].adpcm_value;
                        channel[i].adpcm_loopstart_index = channel[i].adpcm_index;
                    }
                }

                // check if we are at the end of a loop
                if (channel[i].internal_address == (channel[i].soundsad + (channel[i].soundpnt + channel[i].soundlen) * 4)) {
                    u8 repeat_mode = (channel[i].soundcnt >> 27) & 0x3;

                    switch (repeat_mode) {
                    case 0x1:
                        // go to address in memory using soundpnt
                        channel[i].internal_address = channel[i].soundsad + (channel[i].soundpnt * 4);

                        if (format == 2) {
                            // reload adpcm index and value to ones at loopstart address
                            channel[i].adpcm_value = channel[i].adpcm_loopstart_value;
                            channel[i].adpcm_index = channel[i].adpcm_loopstart_index;
                            channel[i].adpcm_second_sample = false;
                        }
                        break;
                    default:
                        // disable the channel
                        channel[i].soundcnt &= ~(1 << 31);
                        break;
                    }
                }
            }
        }

        // using https://problemkaputt.de/gbatek.htm#dssound 
        // Channel/Mixer Bit-Widths section

        // volume divider
        u8 volume_div = (channel[i].soundcnt >> 8) & 0x3;

        // value of 3 divides by 16 not 8
        if (volume_div == 3) {
            volume_div++;
        }

        data <<= (4 - volume_div);

        // volume factor
        u8 volume_factor = channel[i].soundcnt & 0x7F;

        data = (data << 7) * volume_factor / 128;

        // panning / rounding down / mixer
        u8 panning = (channel[i].soundcnt >> 16) & 0x7F;

        sample_left += ((data << 7) * (128 - panning) / 128) >> 10;
        sample_right += ((data << 7) * panning / 128) >> 10;
    }

    // master volume
    u8 master_volume = soundcnt & 0x7F;

    sample_left = (sample_left << 13) * master_volume / 128 / 64;
    sample_right = (sample_right << 13) * master_volume / 128 / 64;

    // strip fraction
    sample_left >>= 21;
    sample_right >>= 21;

    // add bias
    sample_left += soundbias;
    sample_right += soundbias;

    // clipping
    sample_left = std::clamp<s64>(sample_left, 0, 0x3FF);
    sample_right = std::clamp<s64>(sample_right, 0, 0x3FF);

    sample_left -= 0x200;
    sample_right -= 0x200;

    return (sample_right << 16) | (sample_left & 0xFFFF);
}

void SPU::SetAudioInterface(std::shared_ptr<AudioInterface> interface) {
    if (audio_interface) {
        audio_interface->Close();
    }

    audio_interface = interface;

    audio_interface->Open(this, 32768, 1024, (Callback)AudioCallback);
}

void AudioCallback(SPU* spu, s16* stream, int len) {
    int volume = Settings::Get().volume;

    // no point in computing samples
    if (volume == 0) return;

    int multiplier = (volume * 32) / 100;

    // divide by 2 since we are using 2 channels
    int no_samples = len / sizeof(s16) / 2;

    for (int i = 0; i < no_samples; i++) {
        u32 samples = spu->GenerateSamples();

        *stream++ = (samples & 0xFFFF) * multiplier;
        *stream++ = (samples >> 16) * multiplier;
    }
}