#pragma once
#include <dmg/core/video.h>
#include <dmg/core/cartridge.h>
#include <dmg/common/types.h>

class DMGMemory {
public:
    // read and write functions
    u8 read_byte(u16 addr);
    u16 read_halfword(u16 addr);

    void write_byte(u16 addr, u8 data);
    void write_halfword(u16 addr, u16 data);

    DMGVideo video;
    DMGCartridge cartridge;

private:
    

};