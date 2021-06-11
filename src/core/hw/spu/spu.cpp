#include <core/hw/spu/spu.h>
#include <core/core.h>

SPU::SPU(Core* core) : core(core) {

}

void SPU::Reset() {
    SOUNDCNT = 0;
    SOUNDBIAS = 0;
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
    case 0x02:
        // get 3rd byte of SOUNDCNT for that channel
        return ((channel[channel_index].SOUNDCNT >> 16) & 0xFF);
    case 0x03:
        // get upper byte of SOUNDCNT for that channel
        return ((channel[channel_index].SOUNDCNT >> 24) & 0xFF);
    default:
        log_fatal("[SPU] Unhandled address %08x", addr & 0xF);
    }
}

auto SPU::ReadWord(u32 addr) -> u32 {
    u8 channel_index = (addr >> 4) & 0xF;

    switch (addr & 0xF) {
    case 0x00:
        return channel[channel_index].SOUNDCNT;
    default:
        log_fatal("[SPU] Unhandled address %08x", addr & 0xF);
    }
}

void SPU::WriteByte(u32 addr, u8 data) {
    u8 channel_index = (addr >> 4) & 0xF;
    // check which data in the channel should be changed
    switch (addr & 0xF) {
    case 0x00:
        // write to 1st byte of SOUNDCNT
        channel[channel_index].SOUNDCNT = (channel[channel_index].SOUNDCNT & ~0x000000FF) | data;
        break;
    case 0x02:
        // write to 3rd byte of SOUNDCNT
        channel[channel_index].SOUNDCNT = (channel[channel_index].SOUNDCNT & ~0x00FF0000) | (data << 16);
        break;
    case 0x03:
        // write to 4th byte of SOUNDCNT
        channel[channel_index].SOUNDCNT = (channel[channel_index].SOUNDCNT & ~0xFF000000) | (data << 24);
        break;
    default:
        log_fatal("part of sound channel %02x not handled yet", addr & 0xF);
    }
}

void SPU::WriteHalf(u32 addr, u16 data) {
    u8 channel_index = (addr >> 4) & 0xF;
    // check which data in the channel should be changed
    switch (addr & 0xF) {
    case 0x00:
        // write to lower halfword of SOUNDCNT
        channel[channel_index].SOUNDCNT = (channel[channel_index].SOUNDCNT & ~0xFFFF) | (data & 0xFFFF);
        break;
    case 0x08:
        // write to lower halfword of SOUNDTMR
        channel[channel_index].SOUNDTMR = (channel[channel_index].SOUNDTMR & ~0xFFFF) | (data & 0xFFFF);
        break;
    case 0x0A:
        // write to upper halfword of SOUNDTMR
        channel[channel_index].SOUNDTMR = (channel[channel_index].SOUNDTMR & 0xFFFF) | (data << 16);
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
    case 0x00:
        channel[channel_index].SOUNDCNT = data;
        break;
    case 0x04:
        channel[channel_index].SOUNDSAD = data;
        break;
    case 0x08:
        channel[channel_index].SOUNDTMR = data;
        break;
    case 0x0C:
        channel[channel_index].SOUNDLEN = data;
        break;
    default:
        log_fatal("part of sound channel %02x not handled yet", addr & 0xF);
    }
}