#include <algorithm>
#include "Common/Log.h"
#include "Common/Settings.h"
#include "Core/Hardware/SPU.h"
#include "Core/System.h"

SPU::SPU(System& system) : system(system) {}

SPU::~SPU() {
    if (audio_interface != nullptr) {
        audio_interface->close();
    }
}

void SPU::reset() {
    for (int i = 0; i < 16; i++) {
        channel[i].soundcnt = 0;
        channel[i].soundsad = 0;
        channel[i].soundtmr = 0;
        channel[i].soundpnt = 0;
        channel[i].soundlen = 0;
        channel[i].internal_address = 0;
        channel[i].internal_timer = 0;
        channel[i].adpcm_header = 0;
        channel[i].adpcm_value = 0;
        channel[i].adpcm_index = 0;
        channel[i].adpcm_loopstart_value = 0;
        channel[i].adpcm_loopstart_index = 0;
        channel[i].adpcm_second_sample = 0;
    }

    soundcnt = 0;
    soundbias = 0;
    sndcapcnt.fill(0);
    sndcapdad.fill(0);
    sndcaplen.fill(0);
}

void SPU::build_mmio(MMIO& mmio) {
    int channel_size = 16;

    for (int i = 0; i < 16; i++) {
        mmio.register_mmio<u8>(
            0x04000400 + (channel_size * i),
            mmio.invalid_read<u8>(),
            mmio.complex_write<u8>([this, i](u32, u8 data) {
                channel[i].soundcnt = (channel[i].soundcnt & ~0x000000FF) | data;
            })
        );

        mmio.register_mmio<u16>(
            0x04000400 + (channel_size * i),
            mmio.invalid_read<u16>(),
            mmio.complex_write<u16>([this, i](u32, u16 data) {
                channel[i].soundcnt = (channel[i].soundcnt & ~0xFFFF) | (data & 0xFFFF);
            })
        );

        mmio.register_mmio<u32>(
            0x04000400 + (channel_size * i),
            mmio.direct_read<u32>(&channel[i].soundcnt),
            mmio.complex_write<u32>([this, i](u32, u32 data) {
                if (!(channel[i].soundcnt >> 31) && (data >> 31)) {
                    run_channel(i);
                }

                channel[i].soundcnt = data;
            })
        );

        mmio.register_mmio<u8>(
            0x04000402 + (channel_size * i),
            mmio.complex_read<u8>([this, i](u32) {
                return (channel[i].soundcnt >> 16) & 0xFF;
            }),
            mmio.complex_write<u8>([this, i](u32, u8 data) {
                channel[i].soundcnt = (channel[i].soundcnt & ~0x00FF0000) | (data << 16);
            })
        );

        mmio.register_mmio<u8>(
            0x04000403 + (channel_size * i),
            mmio.complex_read<u8>([this, i](u32) {
                return (channel[i].soundcnt >> 24) & 0xFF;
            }),
            mmio.complex_write<u8>([this, i](u32, u8 data) {
                write_soundcnt(i, (channel[i].soundcnt & ~0xFF000000) | (data << 24));
            })
        );

        mmio.register_mmio<u32>(
            0x04000404 + (channel_size * i),
            mmio.invalid_read<u32>(),
            mmio.direct_write<u32>(&channel[i].soundsad, 0x07FFFFFC)
        );

        mmio.register_mmio<u16>(
            0x04000408 + (channel_size * i),
            mmio.invalid_read<u16>(),
            mmio.direct_write<u16>(&channel[i].soundtmr)
        );

        mmio.register_mmio<u16>(
            0x0400040A + (channel_size * i),
            mmio.invalid_read<u16>(),
            mmio.direct_write<u16>(&channel[i].soundpnt)
        );

        mmio.register_mmio<u16>(
            0x0400040C + (channel_size * i),
            mmio.invalid_read<u16>(),
            mmio.direct_write<u16, u32>(&channel[i].soundlen)
        );

        mmio.register_mmio<u32>(
            0x0400040C + (channel_size * i),
            mmio.invalid_read<u32>(),
            mmio.direct_write<u32>(&channel[i].soundlen, 0x3FFFFF)
        );
    }

    mmio.register_mmio<u8>(
        0x04000500,
        mmio.complex_read<u8>([this](u32) {
            return soundcnt;
        }),
        mmio.complex_write<u8>([this](u32, u8 data) {
            soundcnt = (soundcnt & ~0xff) | data;
        })
    );

    mmio.register_mmio<u8>(
        0x04000501,
        mmio.complex_read<u8>([this](u32) {
            return soundcnt >> 8;
        }),
        mmio.complex_write<u8>([this](u32, u8 data) {
            soundcnt = (soundcnt & 0xff) | (data << 8);
        })
    );

    mmio.register_mmio<u16>(
        0x04000504,
        mmio.direct_read<u16>(&soundbias, 0x3ff),
        mmio.direct_write<u16>(&soundbias, 0x3ff)
    );

    mmio.register_mmio<u8>(
        0x04000508,
        mmio.direct_read<u8>(&sndcapcnt[0]),
        mmio.direct_write<u8>(&sndcapcnt[0])
    );

    mmio.register_mmio<u16>(
        0x04000508,
        mmio.complex_read<u16>([this](u32) {
            return sndcapcnt[0] | (sndcapcnt[1] << 8);
        }),
        mmio.complex_write<u16>([this](u32, u16 data) {
            sndcapcnt[0] = data & 0xFF;
            sndcapcnt[1] = data >> 8;
        })
    );

    mmio.register_mmio<u8>(
        0x04000509,
        mmio.direct_read<u8>(&sndcapcnt[1]),
        mmio.direct_write<u8>(&sndcapcnt[1])
    );

    mmio.register_mmio<u32>(
        0x04000510,
        mmio.invalid_read<u32>(),
        mmio.direct_write<u32>(&sndcapdad[0])
    );

    mmio.register_mmio<u16>(
        0x04000514,
        mmio.invalid_read<u16>(),
        mmio.direct_write<u16>(&sndcaplen[0])
    );

    mmio.register_mmio<u32>(
        0x04000518,
        mmio.invalid_read<u32>(),
        mmio.direct_write<u32>(&sndcapdad[1])
    );

    mmio.register_mmio<u16>(
        0x0400051C,
        mmio.invalid_read<u16>(),
        mmio.direct_write<u16>(&sndcaplen[1])
    );
}

u32 SPU::read_word(u32 addr) {
    u8 index = (addr >> 4) & 0xF;

    switch (addr & 0xF) {
    case 0x0:
        return channel[index].soundcnt;
    default:
        log_fatal("[SPU] Unhandled address %08x", addr & 0xF);
    }
}

void SPU::write_word(u32 addr, u32 data) {
    u8 index = (addr >> 4) & 0xF;
    
    switch (addr & 0xF) {
    case 0x0:
        write_soundcnt(index, data);
        break;
    case 0x4:
        channel[index].soundsad = data & 0x07FFFFFC;
        break;
    case 0x8:
        channel[index].soundtmr = data;
        break;
    case 0xC:
        channel[index].soundlen = data & 0x3FFFFF;
        break;
    default:
        log_fatal("part of sound channel %02x not handled yet", addr & 0xF);
    }
}

void SPU::write_soundcnt(int index, u32 data) {
    // start the channel
    if (!(channel[index].soundcnt >> 31) && (data >> 31)) {
        run_channel(index);
    }

    channel[index].soundcnt = data;
}

void SPU::run_channel(int index) {
    // reload internal registers
    channel[index].internal_address = channel[index].soundsad;
    channel[index].internal_timer = channel[index].soundtmr;

    u8 format = (channel[index].soundcnt >> 29) & 0x3;

    switch (format) {
    case 0x2:
        // read in the adpcm header
        channel[index].adpcm_header = system.arm7.memory().read<u32>(channel[index].internal_address);
        channel[index].adpcm_value = (s16)channel[index].adpcm_header;
        channel[index].adpcm_index = std::min((channel[index].adpcm_header >> 16) & 0x7F, 88U);
        channel[index].internal_address += 4;
        break;
    case 0x3:
        // log_warn("handle psg/noise");
        break;
    }
}

u32 SPU::generate_samples() {
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
        case 0x0:
            data = static_cast<s8>(system.arm7.memory().read<u8>(channel[i].internal_address)) << 8;
            data_size = 1;
            break;
        case 0x1:
            data = static_cast<s16>(system.arm7.memory().read<u16>(channel[i].internal_address));
            data_size = 2;
            break;
        case 0x2:
            data = channel[i].adpcm_value;
            break;
        default:
            log_warn("SPU: Handle format %d", format);
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
                    u8 adpcm_data = system.arm7.memory().read<u8>(channel[i].internal_address);

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

void SPU::set_audio_interface(std::shared_ptr<AudioInterface> interface) {
    if (audio_interface) {
        audio_interface->close();
    }

    audio_interface = interface;

    audio_interface->configure(this, 32768, 1024, (AudioCallback)audio_callback);
}

void audio_callback(SPU* spu, s16* stream, int len) {
    int volume = Settings::Get().volume;
    int multiplier = (volume * 32) / 100;
    int no_samples = len / sizeof(s16) / 2;

    for (int i = 0; i < no_samples; i++) {
        u32 samples = spu->generate_samples();
        *stream++ = (samples & 0xFFFF) * multiplier;
        *stream++ = (samples >> 16) * multiplier;
    }
}