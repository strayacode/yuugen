#pragma once

#include <array>
#include "common/types.h"

namespace core {

class SPU {
public:
    void reset();

    u16 read_soundcnt() { return soundcnt; }

    void write_soundcnt(u16 value, u32 mask);
    void write_soundbias(u32 value, u32 mask);
    void write_channel(u32 addr, u32 value, u32 mask);

private:
    void write_channel_control(int index, u32 value, u32 mask);
    void write_channel_source(int index, u32 value, u32 mask);
    void write_channel_timer(int index, u32 value, u32 mask);
    void write_channel_loopstart(int index, u32 value, u32 mask);
    void write_channel_length(int index, u32 value, u32 mask);

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
                u32 repeat : 2;
                u32 format : 2;
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

    std::array<Channel, 16> channels;
    u32 soundbias;
    u16 soundcnt;
};

} // namespace core