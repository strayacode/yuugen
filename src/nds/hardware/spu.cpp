#include <array>
#include <algorithm>
#include "common/bits.h"
#include "common/logger.h"
#include "nds/hardware/spu.h"

namespace nds {

void audio_callback(SPU* spu, s16* stream, int len) {
    int no_samples = len / sizeof(s16) / 2;
    spu->fetch_samples(stream, no_samples);
}

SPU::SPU(common::Scheduler& scheduler, arm::Memory& memory) : scheduler(scheduler), memory(memory) {}

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
        LOG_TODO("unhandled register %02x", addr & 0xf);
    }

    return 0;
}

void SPU::write_sound_capture_control(int id, u8 value) {
    const auto old_control = sound_capture_channels[id].control;
    sound_capture_channels[id].control.data = value;

    if (!old_control.start && sound_capture_channels[id].control.start) {
        sound_capture_channels[id].internal_source = sound_capture_channels[id].destination;
        sound_capture_channels[id].internal_timer = channels[1 + (id * 2)].timer;

        if (sound_capture_channels[id].control.channel_control) {
            LOG_TODO("handle adding sound capture to regular channel");
        }

        if (sound_capture_channels[id].control.source_selection) {
            LOG_TODO("handle using channel 0/2 for capture source");
        }
    }
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
        LOG_TODO("unhandled register %02x", addr & 0xf);
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
        channel.adpcm_second_sample = false;
        channel.internal_source += 4;
        break;
    case AudioFormat::Noise:
        LOG_WARN("handle noise playback");
    }
}

void SPU::play_sample() {
    std::array<s64, 2> mixer;
    std::array<s64, 2> channel1;
    std::array<s64, 2> channel3;
    mixer.fill(0);
    channel1.fill(0);
    channel3.fill(0);

    for (int i = 0; i < 16; i++) {
        auto& channel = channels[i];
        if (!channel.control.start) {
            continue;
        }

        // read sample data
        s64 data = 0;
        switch (channel.control.format) {
        case AudioFormat::PCM8:
            data = common::sign_extend<s64, 16>(static_cast<s8>(memory.read<u8, arm::Bus::System>(channel.internal_source)) << 8);
            break;
        case AudioFormat::PCM16:
            data = static_cast<s16>(memory.read<u16, arm::Bus::System>(channel.internal_source));
            break;
        case AudioFormat::ADPCM:
            data = channel.adpcm_value;
            break;
        case AudioFormat::Noise:
            LOG_WARN("handle noise playback");
        }

        // increment the timer
        // each sample takes up 512 system cycles
        channel.internal_timer += 512;
        while (channel.internal_timer < 512) {
            channel.internal_timer += channel.timer;
                
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
                LOG_WARN("handle noise playback");
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

        // apply volume divider
        u32 divider = channel.control.divider;
        if (divider == 3) {
            divider++;
        }

        data <<= 4 - divider;

        // apply volume factor
        u32 factor = channel.control.factor;
        if (factor == 127) {
            factor++;
        }

        data = (data << 7) * factor / 128;

        // apply panning
        u32 panning = channel.control.panning;
        if (panning == 127) {
            panning++;
        }

        std::array<s64, 2> panned_data;
        panned_data[0] = (data * (128 - panning) / 128) >> 3;
        panned_data[1] = (data * panning / 128) >> 3;

        // store channel 1 and 3 samples for later use
        if (i == 1) {
            channel1[0] = panned_data[0];
            channel1[1] = panned_data[1];

            if (soundcnt.skip_ch1_mixer_output) {
                continue;
            }
        } else if (i == 3) {
            channel3[0] = panned_data[0];
            channel3[1] = panned_data[1];

            if (soundcnt.skip_ch3_mixer_output) {
                continue;
            }
        }

        mixer[0] += panned_data[0];
        mixer[1] += panned_data[1];
    }

    for (int i = 0; i < 2; i++) {
        auto& capture_channel = sound_capture_channels[i];
        auto& channel = channels[1 + (i * 2)];
        if (!capture_channel.control.start) {
            continue;
        }

        // increment the timer
        // each sample takes up 512 system cycles
        capture_channel.internal_timer += 512;
        while (capture_channel.internal_timer < 512) {
            capture_channel.internal_timer += channel.timer;
                
            // get sample from mixer left/right clamped
            s64 sample = std::clamp<s64>(mixer[i], -0x800000, 0x7fffff);

            if (capture_channel.control.format) {
                // pcm8
                memory.write<u8, arm::Bus::System>(capture_channel.internal_source, sample >> 16);
                capture_channel.internal_source++;
            } else {
                // pcm16
                memory.write<u16, arm::Bus::System>(capture_channel.internal_source, sample >> 8);
                capture_channel.internal_source += 2;
            }
        }

        if (capture_channel.internal_source >= capture_channel.destination + (capture_channel.length * 4)) {
            if (capture_channel.control.repeat) {
                // one-shot
                capture_channel.control.start = false;
            } else {
                // loop
                capture_channel.internal_source = capture_channel.destination;
            }
        }
    }

    std::array<s64, 2> samples;
    switch (soundcnt.left_output) {
    case SampleOutput::Mixer:
        samples[0] = mixer[0];
        break;
    case SampleOutput::Channel1:
        samples[0] = channel1[0];
        break;
    case SampleOutput::Channel3:
        samples[0] = channel3[0];
        break;
    case SampleOutput::Channel1And3:
        samples[0] = channel1[0] + channel3[0];
        break;
    }

    switch (soundcnt.right_output) {
    case SampleOutput::Mixer:
        samples[1] = mixer[1];
        break;
    case SampleOutput::Channel1:
        samples[1] = channel1[1];
        break;
    case SampleOutput::Channel3:
        samples[1] = channel3[1];
        break;
    case SampleOutput::Channel1And3:
        samples[1] = channel1[1] + channel3[1];
        break;
    }

    // apply master volume
    u16 master_volume = soundcnt.master_volume;
    if (master_volume == 127) {
        master_volume++;
    }

    samples[0] = (samples[0] * master_volume / 128) >> 8;
    samples[1] = (samples[1] * master_volume / 128) >> 8;

    // apply soundbias
    samples[0] = (samples[0] >> 6) + soundbias;
    samples[1] = (samples[1] >> 6) + soundbias;

    // apply clamping
    samples[0] = std::clamp<s64>(samples[0], 0, 0x3ff);
    samples[1] = std::clamp<s64>(samples[1], 0, 0x3ff);

    samples[0] = (samples[0] - 0x200) << 5;
    samples[1] = (samples[1] - 0x200) << 5;
    
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

} // namespace nds