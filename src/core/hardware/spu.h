#pragma once

#include <array>
#include <memory>
#include <vector>
#include <mutex>
#include "common/audio_device.h"
#include "common/types.h"
#include "core/scheduler.h"
#include "arm/memory.h"

namespace core {

class SPU {
public:
    SPU(Scheduler& scheduler, arm::Memory& memory);
    ~SPU();

    void reset();
    void set_audio_device(std::shared_ptr<common::AudioDevice> audio_device);

    u16 read_soundcnt() { return soundcnt.data; }
    u32 read_channel(u32 addr);
    u8 read_sound_capture_control(int id) { return sound_capture_channels[id].control.data; }
    void write_sound_capture_control(int id, u8 value);
    void write_sound_capture_destination(int id, u32 value, u32 mask);
    void write_sound_capture_length(int id, u32 value, u32 mask);

    void write_soundcnt(u16 value, u32 mask);
    void write_soundbias(u16 value, u32 mask);
    void write_channel(u32 addr, u32 value, u32 mask);

    void fetch_samples(s16* stream, int no_samples);

private:
    void write_channel_control(int id, u32 value, u32 mask);
    void write_channel_source(int id, u32 value, u32 mask);
    void write_channel_timer(int id, u32 value, u32 mask);
    void write_channel_loopstart(int id, u32 value, u32 mask);
    void write_channel_length(int id, u32 value, u32 mask);

    void start_channel(int id);
    void play_sample();
    void next_sample_adpcm(int id);

    enum RepeatMode : u32 {
        Manual = 0,
        Loop = 1,
        OneShot = 2,
        Prohibited = 3,
    };

    enum AudioFormat : u32 {
        PCM8 = 0,
        PCM16 = 1,
        ADPCM = 2,
        Noise = 3,
    };

    struct Channel {
        union Control {
            struct {
                u32 factor : 7;
                u32 : 1;
                u32 divider : 2;
                u32 : 5;
                bool hold : 1;
                u32 panning : 7;
                u32 : 1;
                u32 duty : 3;
                RepeatMode repeat_mode : 2;
                AudioFormat format : 2;
                bool start : 1;
            };

            u32 data;
        };

        Control control;
        u32 source;
        u32 internal_source;
        u16 timer;
        u16 internal_timer;
        u16 loopstart;
        u32 length;
        
        u32 adpcm_header;
        s16 adpcm_value;
        int adpcm_index;
        s16 adpcm_loopstart_value;
        int adpcm_loopstart_index;
        bool adpcm_second_sample;
    };

    struct SoundCaptureChannel {
        union Control {
            struct {
                bool channel_control : 1;
                bool source_selection : 1;
                bool repeat : 1;
                bool format : 1;
                u8 : 3;
                bool start : 1;
            };

            u8 data;
        };

        Control control;
        u32 destination;
        u32 length;
    };

    enum SampleOutput : u16 {
        Mixer = 0,
        Channel1 = 1,
        Channel3 = 2,
        Channel1And3 = 3,
    };

    union SOUNDCNT {
        struct {
            u16 master_volume : 7;
            u16 : 1;
            SampleOutput left_output : 2;
            SampleOutput right_output : 2;
            bool skip_ch1_mixer_output : 1;
            bool skip_ch3_mixer_output : 1;
            u16 : 1;
            bool master_enable : 1;
        };

        u16 data;
    };

    std::array<Channel, 16> channels;
    u16 soundbias;
    SOUNDCNT soundcnt;
    std::array<SoundCaptureChannel, 2> sound_capture_channels;
    EventType play_sample_event;
    std::shared_ptr<common::AudioDevice> audio_device;

    static constexpr int BUFFER_CAPACITY = 8192;

    std::mutex buffer_mutex;
    std::array<u32, BUFFER_CAPACITY> buffer;
    int buffer_size;
    int buffer_head;
    int buffer_tail;

    Scheduler& scheduler;
    arm::Memory& memory;

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

} // namespace core