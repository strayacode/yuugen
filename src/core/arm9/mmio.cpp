#include "common/logger.h"
#include "arm/arch.h"
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
    u32 value = 0;
    switch (MMIO(addr)) {
    case MMIO(0x04000004):
        if constexpr (mask & 0xffff) value |= system.video_unit.read_dispstat(arm::Arch::ARMv5);
        if constexpr (mask & 0xffff0000) logger.error("ARM9Memory: handle vcount read");
        return value;
    case MMIO(0x04000130):
        if constexpr (mask & 0xffff) value |= system.input.read_keyinput();
        if constexpr (mask & 0xffff0000) logger.error("ARM9Memory: handle keycnt read");
        return value;
    case MMIO(0x04000180):
        return system.ipc.read_ipcsync(arm::Arch::ARMv5);
    case MMIO(0x04000184):
        return system.ipc.read_ipcfifocnt(arm::Arch::ARMv5);
    case MMIO(0x04000208):
        return system.arm9.get_irq().read_ime();
    case MMIO(0x04000210):
        return system.arm9.get_irq().read_ie();
    case MMIO(0x04000214):
        return system.arm9.get_irq().read_irf();
    case MMIO(0x04000240):
        if constexpr (mask & 0xff) value |= system.video_unit.vram.read_vramcnt(VRAM::Bank::A);
        if constexpr (mask & 0xff00) value |= system.video_unit.vram.read_vramcnt(VRAM::Bank::B) << 8;
        if constexpr (mask & 0xff0000) value |= system.video_unit.vram.read_vramcnt(VRAM::Bank::C) << 16;
        if constexpr (mask & 0xff000000) value |= system.video_unit.vram.read_vramcnt(VRAM::Bank::D) << 24;
        return value;
    case MMIO(0x04000280):
        return system.maths_unit.read_divcnt();
    case MMIO(0x040002a0):
        return system.maths_unit.read_div_result();
    case MMIO(0x040002a4):
        return system.maths_unit.read_div_result() >> 32;
    case MMIO(0x040002a8):
        return system.maths_unit.read_divrem_result();
    case MMIO(0x040002ac):
        return system.maths_unit.read_divrem_result() >> 32;
    case MMIO(0x040002b0):
        return system.maths_unit.read_sqrtcnt();
    case MMIO(0x040002b4):
        return system.maths_unit.read_sqrt_result();
    case MMIO(0x04004000):
    case MMIO(0x04004008):
        // dsi registers
        return 0;
    case MMIO(0x04100000):
        return system.ipc.read_ipcfiforecv(arm::Arch::ARMv5);
    default:
        logger.warn("ARM9Memory: unmapped %d-bit read %08x", get_access_size(mask), addr + get_access_offset(mask));
        break;
    }

    return 0;
}

// TODO: apply mask to write handlers
template <u32 mask>
void ARM9Memory::mmio_write_word(u32 addr, u32 value) {
    switch (MMIO(addr)) {
    case MMIO(0x04000000):
        system.video_unit.ppu_a.write_dispcnt(value, mask);
        break;
    case MMIO(0x04000004):
        if constexpr (mask & 0xffff) system.video_unit.write_dispstat(arm::Arch::ARMv5, value, mask);
        if constexpr (mask & 0xffff0000) logger.error("ARM9Memory: handle vcount writes");
        break;
    case MMIO(0x040000d0):
        if constexpr (mask & 0xffff) system.dma9.write_length(2, value, mask);
        if constexpr (mask & 0xffff0000) system.dma9.write_control(2, value >> 16, mask >> 16);
        break;
    case MMIO(0x04000180):
        if constexpr (mask & 0xffff) system.ipc.write_ipcsync(arm::Arch::ARMv5, value);
        break;
    case MMIO(0x04000184):
        if constexpr (mask & 0xffff) system.ipc.write_ipcfifocnt(arm::Arch::ARMv5, value);
        break;
    case MMIO(0x04000188):
        system.ipc.write_ipcfifosend(arm::Arch::ARMv5, value);
        break;
    case MMIO(0x04000208):
        system.arm9.get_irq().write_ime(value);
        break;
    case MMIO(0x04000210):
        system.arm9.get_irq().write_ie(value);
        break;
    case MMIO(0x04000214):
        system.arm9.get_irq().write_irf(value);
        break;
    case MMIO(0x04000240):
        if constexpr (mask & 0xff) system.video_unit.vram.write_vramcnt(VRAM::Bank::A, value);
        if constexpr (mask & 0xff00) system.video_unit.vram.write_vramcnt(VRAM::Bank::B, value >> 8);
        if constexpr (mask & 0xff0000) system.video_unit.vram.write_vramcnt(VRAM::Bank::C, value >> 16);
        if constexpr (mask & 0xff000000) system.video_unit.vram.write_vramcnt(VRAM::Bank::D, value >> 24);
        break;
    case MMIO(0x04000244):
        if constexpr (mask & 0xff) system.video_unit.vram.write_vramcnt(VRAM::Bank::E, value);
        if constexpr (mask & 0xff00) system.video_unit.vram.write_vramcnt(VRAM::Bank::F, value >> 8);
        if constexpr (mask & 0xff0000) system.video_unit.vram.write_vramcnt(VRAM::Bank::G, value >> 16);
        if constexpr (mask & 0xff000000) system.write_wramcnt(value >> 24);
        break;
    case MMIO(0x04000248):
        if constexpr (mask & 0xff) system.video_unit.vram.write_vramcnt(VRAM::Bank::H, value);
        if constexpr (mask & 0xff00) system.video_unit.vram.write_vramcnt(VRAM::Bank::I, value >> 8);
        break;
    case MMIO(0x04000280):
        system.maths_unit.write_divcnt(value, mask);
        break;
    case MMIO(0x04000290):
        system.maths_unit.write_div_numer(value, mask);
        break;
    case MMIO(0x04000294):
        system.maths_unit.write_div_numer(static_cast<u64>(value) << 32, static_cast<u64>(mask) << 32);
        break;
    case MMIO(0x04000298):
        system.maths_unit.write_div_denom(value, mask);
        break;
    case MMIO(0x0400029c):
        system.maths_unit.write_div_denom(static_cast<u64>(value) << 32, static_cast<u64>(mask) << 32);
        break;
    case MMIO(0x040002b0):
        system.maths_unit.write_sqrtcnt(value, mask);
        break;
    case MMIO(0x040002b8):
        system.maths_unit.write_sqrt_param(value, mask);
        break;
    case MMIO(0x040002bc):
        system.maths_unit.write_sqrt_param(static_cast<u64>(value) << 32, static_cast<u64>(mask) << 32);
        break;
    case MMIO(0x04000300):
        if constexpr (mask & 0xff) postflg = value & 0x1;
        break;
    case MMIO(0x04000304):
        system.video_unit.write_powcnt1(value);
        break;
    default:
        logger.warn("ARM9Memory: unmapped %d-bit write %08x = %08x", get_access_size(mask), addr + get_access_offset(mask), (value & mask) >> (get_access_offset(mask) * 8));
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