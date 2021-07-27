#include <core/hw/spu/spu.h>
#include <core/hw/hw.h>

// sound notes:
// for pcm audio data,
// pcm8 data NN will have
// the same volume as pcm16 data
// NN00

SPU::SPU(HW* hw) : hw(hw) {

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
        WriteSOUNDCNT(channel_index, (channel[channel_index].soundcnt & ~0x000000FF) | data);
        break;
    case REG_SOUNDCNT + 2:
        // write to 3rd byte of SOUNDCNT
        WriteSOUNDCNT(channel_index, (channel[channel_index].soundcnt & ~0x00FF0000) | (data << 16));
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
        WriteSOUNDCNT(channel_index, (channel[channel_index].soundcnt & ~0xFFFF) | (data & 0xFFFF));
        break;
    case REG_SOUNDTMR:
        // write to lower halfword of SOUNDTMR
        channel[channel_index].soundtmr = data;
        break;
    case REG_SOUNDPNT:
        // write to SOUNDPNT
        channel[channel_index].soundpnt = data;
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
        channel[channel_index].soundsad = data;
        break;
    case REG_SOUNDTMR:
        channel[channel_index].soundtmr = data;
        break;
    case REG_SOUNDLEN:
        channel[channel_index].soundlen = data;
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
    log_fatal("run channel %d lol", channel_index);
}

void SPU::RunMixer() {
    for (int i = 0; i < 16; i++) {
        // don't mix audio from channels
        // that are disabled
        if (!(channel[i].soundcnt >> 31)) {
            continue;
        }
    }
}