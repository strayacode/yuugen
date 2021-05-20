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

void SPU::WriteByte(u32 addr, u8 data) {
    // do later lol
}

void SPU::WriteHalf(u32 addr, u16 data) {
    // do later lol
}

void SPU::WriteWord(u32 addr, u32 data) {
    // if (data != 0) {
    //     log_fatal("implement sound channel support for spu");
    // }
    // do later lol
    // // get the channel index
    // u8 channel_index = (addr >> 4) & 0xF;
    // // check which data in the channel should be changed
    // switch (addr & 0xF) {
    // default:
    //     log_fatal("part of sound channel %02x not handled yet", addr & 0xF);
    // }
}