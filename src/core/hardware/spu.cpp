#include "common/logger.h"
#include "core/hardware/spu.h"

namespace core {

SPU::SPU(Scheduler& scheduler) : scheduler(scheduler) {}

void SPU::reset() {
    channels.fill(Channel{});
    soundbias = 0;
}

void SPU::write_soundcnt(u16 value, u32 mask) {
    soundcnt = (soundcnt & ~mask) | (value & mask);
}

void SPU::write_soundbias(u32 value, u32 mask) {
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
        write_channel_timer(id, value, mask);
        break;
    case 0xa:
        write_channel_loopstart(id, value, mask);
        break;
    case 0xc:
        write_channel_length(id, value, mask);
        break;
    default:
        logger.todo("SPU: unhandled register %02x", addr & 0xf);
    }
}

void SPU::write_channel_control(int id, u32 value, u32 mask) {
    auto& channel = channels[id];
    auto old_control = channel.control;
    channel.control.data = (channel.control.data & ~mask) | (value & mask);

    if (!old_control.start && channel.control.start) {
        logger.todo("SPU: start spu channel");
    }

    if (old_control.start && !channel.control.start) {
        logger.todo("SPU: handle stopping spu channel");
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
    case AudioFormat::ADPCM:
        logger.error("SPU: handle adpcm playback");
    case AudioFormat::Noise:
        logger.error("SPU: handle noise playback");
    }
}

} // namespace core