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

    void reset();
    void set_audio_device(std::shared_ptr<common::AudioDevice> audio_device);

    u16 read_soundcnt() { return soundcnt.data; }
    u32 read_channel(u32 addr);
    u8 read_soundcapcnt(u32 addr) { return soundcapcnt[addr - 0x04000508]; }

    void write_soundcnt(u16 value, u32 mask);
    void write_soundbias(u32 value, u32 mask);
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
                u32 multiplier : 7;
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
        s16 adpcm_loopstart_index;
        bool adpcm_second_sample;
    };

    union SOUNDCNT {
        struct {
            u16 master_volume : 7;
            u16 : 1;
            u16 left_output : 2;
            u16 right_output : 2;
            bool output_ch1_to_mixer : 1;
            bool output_ch3_to_mixer : 1;
            u16 : 1;
            bool master_enable : 1;
        };

        u16 data;
    };

    std::array<Channel, 16> channels;
    u32 soundbias;
    SOUNDCNT soundcnt;
    std::array<u8, 8> soundcapcnt;
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

    static constexpr std::array<int, 8> index_table = {-1, -1, -1, -1, 2, 4, 6, 8};
    static constexpr std::array<int, 89> adpcm_table = {
        0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x0010, 0x0011,
        0x0013, 0x0015, 0x0017, 0x0019, 0x001c, 0x001f, 0x0022, 0x0025, 0x0029, 0x002d,
        0x0032, 0x0037, 0x003c, 0x0042, 0x0049, 0x0050, 0x0058, 0x0061, 0x006b, 0x0076,
        0x0082, 0x008f, 0x009d, 0x00ad, 0x00be, 0x00d1, 0x00e6, 0x00fd, 0x0117, 0x0133,
        0x0151, 0x0173, 0x0198, 0x01c1, 0x01ee, 0x0220, 0x0256, 0x0292, 0x02d4, 0x031c,
        0x036c, 0x03c3, 0x0424, 0x048e, 0x0502, 0x0583, 0x0610, 0x06ab, 0x0756, 0x0812,
        0x08e0, 0x09c3, 0x0abd, 0x0bd0, 0x0cff, 0x0e4c, 0x0fba, 0x114c, 0x1307, 0x14ee,
        0x1706, 0x1954, 0x1bdc, 0x1ea5, 0x21b6, 0x2515, 0x28ca, 0x2cdf, 0x315b, 0x364b,
        0x3bb9, 0x41b2, 0x4844, 0x4f7e, 0x5771, 0x602f, 0x69ce, 0x7462, 0x7fff
    };
};

} // namespace core