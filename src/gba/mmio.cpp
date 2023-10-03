#include "common/logger.h"
#include "arm/arch.h"
#include "gba/memory.h"
#include "gba/system.h"

namespace gba {

#define MMIO(addr) (addr >> 2)

u8 Memory::mmio_read_byte(u32 addr) {
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

u16 Memory::mmio_read_half(u32 addr) {
    switch (addr & 0x2) {
    case 0x0:
        return mmio_read_word<0x0000ffff>(addr & ~0x2);
    case 0x2:
        return mmio_read_word<0xffff0000>(addr & ~0x2) >> 16;
    }

    return 0;
}

u32 Memory::mmio_read_word(u32 addr) {
    return mmio_read_word<0xffffffff>(addr);
}

void Memory::mmio_write_byte(u32 addr, u8 value) {
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

void Memory::mmio_write_half(u32 addr, u16 value) {
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

void Memory::mmio_write_word(u32 addr, u32 value) {
    mmio_write_word<0xffffffff>(addr, value);
}

template <u32 mask>
u32 Memory::mmio_read_word(u32 addr) {
    u32 value = 0;
    switch (MMIO(addr)) {
    case MMIO(0x04000000):
        return system.ppu.read_dispcnt();
    case MMIO(0x04000004):
        if constexpr (mask & 0xffff) value |= system.ppu.read_dispstat();
        if constexpr (mask & 0xffff0000) value |= system.ppu.read_vcount() << 16;
        return value;
    case MMIO(0x04000008):
        if constexpr (mask & 0xffff) value |= system.ppu.read_bgcnt(0);
        if constexpr (mask & 0xffff0000) value |= system.ppu.read_bgcnt(1) << 16;
        return value;
    case MMIO(0x0400000c):
        if constexpr (mask & 0xffff) value |= system.ppu.read_bgcnt(2);
        if constexpr (mask & 0xffff0000) value |= system.ppu.read_bgcnt(3) << 16;
        return value;
    case MMIO(0x04000130):
        if constexpr (mask & 0xffff) value |= system.input.read_keyinput();
        if constexpr (mask & 0xffff0000) logger.error("gba::Memory: handle keycnt read");
        return value;
    case MMIO(0x04000200):
        if constexpr (mask & 0xffff) value |= system.irq.read_ie();
        if constexpr (mask & 0xffff0000) value |= system.irq.read_irf() << 16;
        return value;
    case MMIO(0x04000208):
        return system.irq.read_ime();
    default:
        logger.todo("Memory: unmapped mmio %d-bit read %08x", get_access_size(mask), addr + get_access_offset(mask));
        break;
    }

    return 0;
}

template <u32 mask>
void Memory::mmio_write_word(u32 addr, u32 value) {
    switch (MMIO(addr)) {
    case MMIO(0x04000000):
        system.ppu.write_dispcnt(value, mask);
        break;
    case MMIO(0x04000004):
        if constexpr (mask & 0xffff) system.ppu.write_dispstat(value, mask);
        if constexpr (mask & 0xffff0000) logger.todo("Memory: handle vcount write");
        break;
    case MMIO(0x04000008):
        if constexpr (mask & 0xffff) system.ppu.write_bgcnt(0, value, mask);
        if constexpr (mask & 0xffff0000) system.ppu.write_bgcnt(1, value >> 16, mask >> 16);
        break;
    case MMIO(0x0400000c):
        if constexpr (mask & 0xffff) system.ppu.write_bgcnt(2, value, mask);
        if constexpr (mask & 0xffff0000) system.ppu.write_bgcnt(3, value >> 16, mask >> 16);
        break;
    case MMIO(0x04000200):
        if constexpr (mask & 0xffff) system.irq.write_ie(value, mask);
        if constexpr (mask & 0xffff0000) system.irq.write_irf(value >> 16, mask >> 16);
        break;
    case MMIO(0x04000208):
        system.irq.write_ime(value, mask);
        break;
    case MMIO(0x04000300):
        if constexpr (mask & 0xff) logger.todo("handle postflg write");
        if constexpr (mask & 0xff00) write_haltcnt(value >> 8);
        break;
    default:
        logger.todo("Memory: unmapped mmio %d-bit write %08x = %08x", get_access_size(mask), addr + get_access_offset(mask), (value & mask) >> (get_access_offset(mask) * 8));
        break;
    }
}

int Memory::get_access_size(u32 mask) {
    int size = 0;
    for (int i = 0; i < 4; i++) {
        if (mask & 0xff) {
            size += 8;
        }

        mask >>= 8;
    }

    return size;
}

u32 Memory::get_access_offset(u32 mask) {
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

} // namespace gba