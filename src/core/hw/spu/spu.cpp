#include <core/hw/spu/spu.h>
#include <core/core.h>

SPU::SPU(Core* core) : core(core) {

}

void SPU::Reset() {
    SOUNDCNT = 0;
}

void SPU::WriteSoundChannel(u32 addr, u32 data) {
    if (data != 0) {
        log_fatal("implement sound channel support for spu");
    }
    // // get the channel index
    // u8 channel_index = (addr >> 4) & 0xF;
    // // check which data in the channel should be changed
    // switch (addr & 0xF) {
    // default:
    //     log_fatal("part of sound channel %02x not handled yet", addr & 0xF);
    // }
}