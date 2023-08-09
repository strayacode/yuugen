#include "common/logger.h"
#include "core/hardware/spu.h"

namespace core {

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
    auto index = (addr >> 4) & 0xf;
    switch (addr & 0xf) {
    case 0x0:
        write_channel_control(index, value, mask);
        break;
    case 0x4:
        write_channel_source(index, value, mask);
        break;
    case 0x8:
        write_channel_timer(index, value, mask);
        break;
    case 0xa:
        write_channel_loopstart(index, value, mask);
        break;
    case 0xc:
        write_channel_length(index, value, mask);
        break;
    default:
        logger.todo("SPU: unhandled register %02x", addr & 0xf);
    }
}

void SPU::write_channel_control(int index, u32 value, u32 mask) {
    auto& channel = channels[index];
    auto old_control = channel.control;
    channel.control.data = (channel.control.data & ~mask) | (value & mask);

    if (!old_control.start && channel.control.start) {
        logger.todo("SPU: start spu channel");
    }
}

void SPU::write_channel_source(int index, u32 value, u32 mask) {
    mask &= 0x07fffffc;
    channels[index].source = (channels[index].source & ~mask) | (value & mask);
}

void SPU::write_channel_timer(int index, u32 value, u32 mask) {
    channels[index].timer = (channels[index].timer & ~mask) | (value & mask);
}

void SPU::write_channel_loopstart(int index, u32 value, u32 mask) {
    channels[index].loopstart = (channels[index].loopstart & ~mask) | (value & mask);
}

void SPU::write_channel_length(int index, u32 value, u32 mask) {
    mask &= 0x3fffff;
    channels[index].length = (channels[index].length & ~mask) | (value & mask);
}

} // namespace core