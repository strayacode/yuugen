#include <array>
#include <algorithm>
#include "common/bits.h"
#include "common/logger.h"
#include "core/hardware/spu.h"

namespace core {

void audio_callback(SPU* spu, s16* stream, int len) {
    int no_samples = len / sizeof(s16) / 2;
    spu->fetch_samples(stream, no_samples);
}

SPU::SPU(Scheduler& scheduler, arm::Memory& memory) : scheduler(scheduler), memory(memory) {}

SPU::~SPU() {
    if (audio_device != nullptr) {
        audio_device->close();
    }
}

void SPU::reset() {
    channels.fill(Channel{});
    soundbias = 0;
    soundcnt.data = 0;
    sound_capture_channels.fill(SoundCaptureChannel{});
    
    buffer.fill(0);
    buffer_size = 0;
    buffer_head = 0;
    buffer_tail = 0;

    play_sample_event = scheduler.register_event("SPU Sample", [this]() {
        play_sample();
        scheduler.add_event(1024, &play_sample_event);
    });

    scheduler.add_event(1024, &play_sample_event);
}

void SPU::set_audio_device(std::shared_ptr<common::AudioDevice> audio_device) {
    if (audio_device) {
        audio_device->close();
    }

    this->audio_device = audio_device;
    this->audio_device->configure(this, 32768, 1024, (common::AudioCallback)audio_callback);
}

u32 SPU::read_channel(u32 addr) {
    auto id = (addr >> 4) & 0xf;
    switch (addr & 0xf) {
    case 0x0:
        return channels[id].control.data;
    default:
        logger.todo("SPU: unhandled register %02x", addr & 0xf);
    }

    return 0;
}

void SPU::write_sound_capture_control(int id, u8 value) {
    sound_capture_channels[id].control.data = value;
}

void SPU::write_sound_capture_destination(int id, u32 value, u32 mask) {
    mask &= 0x7fffffc;
    sound_capture_channels[id].destination = (sound_capture_channels[id].destination & ~mask) | (value & mask);
}

void SPU::write_sound_capture_length(int id, u32 value, u32 mask) {
    mask &= 0xffff;
    sound_capture_channels[id].length = (sound_capture_channels[id].length & ~mask) | (value & mask);
}

void SPU::write_soundcnt(u16 value, u32 mask) {
    soundcnt.data = (soundcnt.data & ~mask) | (value & mask);
}

void SPU::write_soundbias(u16 value, u32 mask) {
    mask &= 0x3ff;
    soundbias = (soundbias & ~mask) | (value & mask);
}

void SPU::write_channel(u32 addr, u32 value, u32 mask) {
    auto id = (addr >> 4) & 0xf;
    switch (addr & 0xf) {
    case 0x0:
        write_channel_control(id, value, mask);
        break;
    case 0x4:
        write_channel_source(id, value, mask);
        break;
    case 0x8:
        if (mask & 0xffff) write_channel_timer(id, value, mask);
        if (mask & 0xffff0000) write_channel_loopstart(id, value >> 16, mask >> 16);
        break;
    case 0xc:
        write_channel_length(id, value, mask);
        break;
    default:
        logger.todo("SPU: unhandled register %02x", addr & 0xf);
    }
}

void SPU::fetch_samples(s16* stream, int no_samples) {
    std::lock_guard<std::mutex> buffer_lock{buffer_mutex};

    if (buffer_size < no_samples) {
        for (int i = 0; i < no_samples; i++) {
            u32 sample = buffer[(buffer_head + i) % BUFFER_CAPACITY];
            *stream++ = sample & 0xffff;
            *stream++ = sample >> 16;
        }
    } else {
        for (int i = 0; i < no_samples; i++) {
            u32 sample = buffer[buffer_head];
            *stream++ = sample & 0xffff;
            *stream++ = sample >> 16;
            buffer_head = (buffer_head + 1) % BUFFER_CAPACITY;
            buffer_size--;
        }
    }
}

void SPU::write_channel_control(int id, u32 value, u32 mask) {
    auto& channel = channels[id];
    auto old_control = channel.control;
    channel.control.data = (channel.control.data & ~mask) | (value & mask);

    if (!old_control.start && channel.control.start && soundcnt.master_enable) {
        start_channel(id);
    }
}

void SPU::write_channel_source(int id, u32 value, u32 mask) {
    mask &= 0x07fffffc;
    channels[id].source = (channels[id].source & ~mask) | (value & mask);
}

void SPU::write_channel_timer(int id, u32 value, u32 mask) {
    channels[id].timer = (channels[id].timer & ~mask) | (value & mask);
}

void SPU::write_channel_loopstart(int id, u32 value, u32 mask) {
    channels[id].loopstart = (channels[id].loopstart & ~mask) | (value & mask);
}

void SPU::write_channel_length(int id, u32 value, u32 mask) {
    mask &= 0x3fffff;
    channels[id].length = (channels[id].length & ~mask) | (value & mask);
}

void SPU::start_channel(int id) {
    auto& channel = channels[id];
    channel.internal_source = channel.source;
    channel.internal_timer = channel.timer;

    switch (channel.control.format) {
    case AudioFormat::PCM8:
    case AudioFormat::PCM16:
        break;
    case AudioFormat::ADPCM:
        channel.adpcm_header = memory.read<u32, arm::Bus::System>(channel.internal_source);
        channel.adpcm_value = static_cast<s16>(channel.adpcm_header);
        channel.adpcm_index = std::min(common::get_field<16, 7>(channel.adpcm_header), static_cast<u32>(88));
        channel.internal_source += 4;
        break;
    case AudioFormat::Noise:
        logger.error("SPU: handle noise playback");
    }
}

void SPU::play_sample() {
    // TODO: handle left/right channel and mixer
    std::array<s64, 2> samples;
    samples.fill(0);

    for (int i = 0; i < 16; i++) {
        auto& channel = channels[i];
        if (!channel.control.start) {
            continue;
        }

        // read sample data
        s64 data = 0;
        switch (channel.control.format) {
        case AudioFormat::PCM8:
            data = static_cast<s8>(memory.read<u8, arm::Bus::System>(channel.internal_source)) << 8;
            break;
        case AudioFormat::PCM16:
            data = static_cast<s16>(memory.read<u16, arm::Bus::System>(channel.internal_source));
            break;
        case AudioFormat::ADPCM:
            data = channel.adpcm_value;
            break;
        case AudioFormat::Noise:
            logger.error("SPU: handle noise playback");
        }

        // increment the timer
        // each sample takes up 512 system cycles
        for (int j = 0; j < 512; j++) {
            channel.internal_timer++;
            if (channel.internal_timer == 0) {
                channel.internal_timer = channel.timer;
                
                switch (channel.control.format) {
                case AudioFormat::PCM8:
                    channel.internal_source++;
                    break;
                case AudioFormat::PCM16:
                    channel.internal_source += 2;
                    break;
                case AudioFormat::ADPCM:
                    next_sample_adpcm(i);
                    break;
                case AudioFormat::Noise:
                    logger.error("SPU: handle noise playback");
                }

                if (channel.internal_source == (channel.source + (channel.loopstart + channel.length) * 4)) {
                    if (channel.control.repeat_mode == RepeatMode::Loop) {
                        channel.internal_source = channel.source + (channel.loopstart * 4);

                        if (channel.control.format == AudioFormat::ADPCM) {
                            channel.adpcm_value = channel.adpcm_loopstart_value;
                            channel.adpcm_index = channel.adpcm_loopstart_index;
                            channel.adpcm_second_sample = false;
                        }
                    } else {
                        channel.control.start = false;
                    }
                }
            }
        }

        auto divider = channel.control.divider;
        if (divider == 3) {
            divider++;
        }

        data <<= 4 - divider;
        data = (data << 7) * channel.control.factor / 128;

        samples[0] += ((data << 7) * (128 - channel.control.panning) / 128) >> 10;
        samples[1] += ((data << 7) * channel.control.panning / 128) >> 10;
    }

    samples[0] = (samples[0] << 13) * soundcnt.master_volume / 128 / 64;
    samples[1] = (samples[1] << 13) * soundcnt.master_volume / 128 / 64;

    samples[0] >>= 21;
    samples[1] >>= 21;

    samples[0] += soundbias;
    samples[1] += soundbias;

    samples[0] = std::clamp<s64>(samples[0], 0, 0x3ff);
    samples[1] = std::clamp<s64>(samples[1], 0, 0x3ff);

    samples[0] -= 0x200;
    samples[1] -= 0x200;

    u32 combined = (samples[1] << 16) | (samples[0] & 0xffff);

    std::lock_guard<std::mutex> buffer_lock{buffer_mutex};

    buffer[buffer_tail] = combined;
    
    if (buffer_size < BUFFER_CAPACITY) {
        buffer_tail = (buffer_tail + 1) % BUFFER_CAPACITY;
        buffer_size++;
    }
}

void SPU::next_sample_adpcm(int id) {
    auto& channel = channels[id];
    u8 data = memory.read<u8, arm::Bus::System>(channel.internal_source);

    if (channel.adpcm_second_sample) {
        data = (data >> 4) & 0xf;
    } else {
        data &= 0xf;
    }

    int diff = adpcm_table[channel.adpcm_index] / 8;

    if (common::get_bit<0>(data)) {
        diff += adpcm_table[channel.adpcm_index] / 4;
    }

    if (common::get_bit<1>(data)) {
        diff += adpcm_table[channel.adpcm_index] / 2;
    }

    if (common::get_bit<2>(data)) {
        diff += adpcm_table[channel.adpcm_index];
    }

    if (common::get_bit<3>(data)) {
        channel.adpcm_value = std::max(channel.adpcm_value - diff, -0x7fff);
    } else {
        channel.adpcm_value = std::min(channel.adpcm_value + diff, 0x7fff);
    }

    channel.adpcm_index = std::clamp(channel.adpcm_index + index_table[common::get_field<0, 3>(data)], 0, 88);
    channel.adpcm_second_sample = !channel.adpcm_second_sample;

    if (!channel.adpcm_second_sample) {
        channel.internal_source++;
    }

    if ((channel.internal_source == channel.source + (channel.loopstart * 4)) && !channel.adpcm_second_sample) {
        channel.adpcm_loopstart_value = channel.adpcm_value;
        channel.adpcm_loopstart_index = channel.adpcm_index;
    }
}

u32 SPU::generate_sample() {
    s64 sample_left = 0;
    s64 sample_right = 0;

    for (int i = 0; i < 16; i++) {
        // don't mix audio from channels
        // that are disabled
        if (!(channels[i].control.data >> 31)) {
            continue;
        }

        s64 data = 0;
        u8 data_size = 0;
        u8 format = (channels[i].control.data >> 29) & 0x3;

        switch (format) {
        case 0x0:
            data = static_cast<s8>(memory.read<u8, arm::Bus::System>(channels[i].internal_source)) << 8;
            data_size = 1;
            break;
        case 0x1:
            data = static_cast<s16>(memory.read<u16, arm::Bus::System>(channels[i].internal_source));
            data_size = 2;
            break;
        case 0x2:
            data = channels[i].adpcm_value;
            break;
        default:
            logger.warn("SPU: Handle format %d", format);
            break;
        }

        // 512 cycles are used up before a sample can be generated
        // TODO: make this correctly timed against the system clock
        for (int j = 0; j < 512; j++) {
            channels[i].internal_timer++;
        
            if (channels[i].internal_timer == 0) {
                // overflow occured
                channels[i].internal_timer = channels[i].timer;
                channels[i].internal_source += data_size;

                if (format == 2) {
                    // decode adpcm data
                    u8 adpcm_data = memory.read<u8, arm::Bus::System>(channels[i].internal_source);

                    // each sample is 4-bit
                    if (channels[i].adpcm_second_sample) {
                        adpcm_data = (adpcm_data >> 4) & 0xF;
                    } else {
                        adpcm_data &= 0xF;
                    }

                    int diff = adpcm_table[channels[i].adpcm_index] / 8;

                    if (adpcm_data & 0x1) {
                        diff += adpcm_table[channels[i].adpcm_index] / 4;
                    }

                    if (adpcm_data & (1 << 1)) {
                        diff += adpcm_table[channels[i].adpcm_index] / 2;
                    }

                    if (adpcm_data & (1 << 2)) {
                        diff += adpcm_table[channels[i].adpcm_index];
                    }

                    if (adpcm_data & (1 << 3)) {
                        channels[i].adpcm_value = std::max(channels[i].adpcm_value - diff, -0x7FFF);
                    } else {
                        channels[i].adpcm_value = std::min(channels[i].adpcm_value + diff, 0x7FFF);
                    }

                    channels[i].adpcm_index = std::clamp(channels[i].adpcm_index + index_table[adpcm_data & 0x7], 0, 88);

                    // toggle between the first and second byte
                    channels[i].adpcm_second_sample = !channels[i].adpcm_second_sample;

                    // go to next byte once a second sample has been generated
                    if (!channels[i].adpcm_second_sample) {
                        channels[i].internal_source++;
                    }

                    // save the value and index if we are at the loopstart location
                    // and are on the first byte
                    if ((channels[i].internal_source == channels[i].source + (channels[i].loopstart * 4)) && !channels[i].adpcm_second_sample) {
                        channels[i].adpcm_loopstart_value = channels[i].adpcm_value;
                        channels[i].adpcm_loopstart_index = channels[i].adpcm_index;
                    }
                }

                // check if we are at the end of a loop
                if (channels[i].internal_source == (channels[i].source + (channels[i].loopstart + channels[i].length) * 4)) {
                    u8 repeat_mode = (channels[i].control.data >> 27) & 0x3;

                    switch (repeat_mode) {
                    case 0x1:
                        // go to address in memory using loopstart
                        channels[i].internal_source = channels[i].source + (channels[i].loopstart * 4);

                        if (format == 2) {
                            // reload adpcm index and value to ones at loopstart address
                            channels[i].adpcm_value = channels[i].adpcm_loopstart_value;
                            channels[i].adpcm_index = channels[i].adpcm_loopstart_index;
                            channels[i].adpcm_second_sample = false;
                        }
                        break;
                    default:
                        // disable the channel
                        channels[i].control.data &= ~(1 << 31);
                        break;
                    }
                }
            }
        }

        // using https://problemkaputt.de/gbatek.htm#dssound 
        // Channel/Mixer Bit-Widths section

        // volume divider
        u8 volume_div = (channels[i].control.data >> 8) & 0x3;

        // value of 3 divides by 16 not 8
        if (volume_div == 3) {
            volume_div++;
        }

        data <<= (4 - volume_div);

        // volume factor
        u8 volume_factor = channels[i].control.data & 0x7F;

        data = (data << 7) * volume_factor / 128;

        // panning / rounding down / mixer
        u8 panning = (channels[i].control.data >> 16) & 0x7F;

        sample_left += ((data << 7) * (128 - panning) / 128) >> 10;
        sample_right += ((data << 7) * panning / 128) >> 10;
    }

    // master volume
    u8 master_volume = soundcnt.data & 0x7F;

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

} // namespace core