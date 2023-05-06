#include "common/logger.h"
#include "core/arm9/arm9.h"
#include "core/system.h"

namespace core {

#define MMIO(addr) (addr >> 2)

u8 ARM9Memory::mmio_read_byte(u32 addr) {
    switch (addr & 0x3) {
    case 0x0:
        return mmio_read_word<0x000000ff>(addr & ~0x3);
    case 0x1:
        return mmio_read_word<0x0000ff00>(addr & ~0x3) >> 8;
    case 0x2:
        return mmio_read_word<0x00ff0000>(addr & ~0x3) >> 16;
    case 0x3:
        return mmio_read_word<0xff000000>(addr & ~0x3) >> 24;
    }

    return 0;
}

u16 ARM9Memory::mmio_read_half(u32 addr) {
    switch (addr & 0x2) {
    case 0x0:
        return mmio_read_word<0x0000ffff>(addr & ~0x2);
    case 0x2:
        return mmio_read_word<0xffff0000>(addr & ~0x2) >> 16;
    }

    return 0;
}

u32 ARM9Memory::mmio_read_word(u32 addr) {
    return mmio_read_word<0xffffffff>(addr);
}

void ARM9Memory::mmio_write_byte(u32 addr, u8 value) {
    u32 mirrored = value * 0x01010101;
    switch (addr & 0x3) {
    case 0x0:
        mmio_write_word<0x000000ff>(addr & ~0x3, mirrored);
        break;
    case 0x1:
        mmio_write_word<0x0000ff00>(addr & ~0x3, mirrored);
        break;
    case 0x2:
        mmio_write_word<0x00ff0000>(addr & ~0x3, mirrored);
        break;
    case 0x3:
        mmio_write_word<0xff000000>(addr & ~0x3, mirrored);
        break;
    }
}

void ARM9Memory::mmio_write_half(u32 addr, u16 value) {
    u32 mirrored = value * 0x00010001;
    switch (addr & 0x2) {
    case 0x0:
        mmio_write_word<0x0000ffff>(addr & ~0x2, mirrored);
        break;
    case 0x2:
        mmio_write_word<0xffff0000>(addr & ~0x2, mirrored);
        break;
    }
}

void ARM9Memory::mmio_write_word(u32 addr, u32 value) {
    mmio_write_word<0xffffffff>(addr, value);
}

template <u32 mask>
u32 ARM9Memory::mmio_read_word(u32 addr) {
    switch (MMIO(addr)) {
    default:
        logger.error("ARM9Memory: unmapped %d-bit read %08x", get_access_size(mask), addr + get_access_offset(mask));
        break;
    }

    return 0;
}

template <u32 mask>
void ARM9Memory::mmio_write_word(u32 addr, u32 value) {
    switch (MMIO(addr)) {
    case MMIO(0x04000000):
        system.video_unit.ppu_a.write_dispcnt(value, mask);
        break;
    case MMIO(0x04000240):
        if constexpr (mask & 0xff) system.video_unit.vram.write_vramcnt(VRAM::Bank::A, value);
        if constexpr (mask & 0xff00) system.video_unit.vram.write_vramcnt(VRAM::Bank::B, value >> 8);
        if constexpr (mask & 0xff0000) system.video_unit.vram.write_vramcnt(VRAM::Bank::C, value >> 16);
        if constexpr (mask & 0xff000000) system.video_unit.vram.write_vramcnt(VRAM::Bank::D, value >> 24);
        break;
    case MMIO(0x04000244):
        if constexpr (mask & 0xff) logger.error("ARM9Memory: handle vramcnt_e write");
        if constexpr (mask & 0xff00) logger.error("ARM9Memory: handle vramcnt_f write");
        if constexpr (mask & 0xff0000) logger.error("ARM9Memory: handle vramcnt_g write");
        if constexpr (mask & 0xff000000) system.write_wramcnt(value >> 24);
        break;
    case MMIO(0x04000300):
        if constexpr (mask & 0xff) postflg = value & 0x1;
        break;
    case MMIO(0x04000304):
        system.video_unit.write_powcnt1(value);
        break;
    default:
        logger.error("ARM9Memory: unmapped %d-bit write %08x = %08x", get_access_size(mask), addr + get_access_offset(mask), (value & mask) >> (get_access_offset(mask) * 8));
        break;
    }
}

int ARM9Memory::get_access_size(u32 mask) {
    int size = 0;
    for (int i = 0; i < 4; i++) {
        if (mask & 0xff) {
            size += 8;
        }

        mask >>= 8;
    }

    return size;
}

u32 ARM9Memory::get_access_offset(u32 mask) {
    u32 offset = 0;
    for (int i = 0; i < 4; i++) {
        if (mask & 0xff) {
            break;
        }

        offset++;
        mask >>= 8;
    }

    return offset;
}

} // namespace core