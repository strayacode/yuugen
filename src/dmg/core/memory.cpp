#include <dmg/core/memory.h>
#include <dmg/common/types.h>
#include <stdio.h>
#include <stdlib.h>

u8 DMGMemory::read_byte(u16 addr) {
    switch (addr & 0xF000) {
    case 0x0000: case 0x1000: case 0x2000: case 0x3000: case 0x4000: case 0x5000: case 0x6000: case 0x7000:
        return cartridge.rom[addr];

    default:
        printf("[Memory] undefined 8 bit read at address 0x%04x!\n", addr);
        exit(1);
    }
}

u16 DMGMemory::read_halfword(u16 addr) {
    return (read_byte(addr + 1) << 8 | read_byte(addr));
}