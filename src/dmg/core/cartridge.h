#pragma once
#include <dmg/common/types.h>

class DMGCartridge {
public:
    void load_bootrom();

    u8 rom[32 * 1024];
    u8 e_ram[8 * 1024];
    u8 rom_cache[0xFF]; // used to preserve the first 256 bytes of rom to load after the bootrom has finished executing

private:

};