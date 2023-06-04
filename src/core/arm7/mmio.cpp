#include "common/logger.h"
#include "arm/arch.h"
#include "core/arm7/arm7.h"
#include "core/system.h"

namespace core {

#define MMIO(addr) (addr >> 2)

u8 ARM7Memory::mmio_read_byte(u32 addr) {
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

u16 ARM7Memory::mmio_read_half(u32 addr) {
    switch (addr & 0x2) {
    case 0x0:
        return mmio_read_word<0x0000ffff>(addr & ~0x2);
    case 0x2:
        return mmio_read_word<0xffff0000>(addr & ~0x2) >> 16;
    }

    return 0;
}

u32 ARM7Memory::mmio_read_word(u32 addr) {
    return mmio_read_word<0xffffffff>(addr);
}

void ARM7Memory::mmio_write_byte(u32 addr, u8 value) {
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

void ARM7Memory::mmio_write_half(u32 addr, u16 value) {
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

void ARM7Memory::mmio_write_word(u32 addr, u32 value) {
    mmio_write_word<0xffffffff>(addr, value);
}

template <u32 mask>
u32 ARM7Memory::mmio_read_word(u32 addr) {
    u32 value = 0;
    switch (MMIO(addr)) {
    case MMIO(0x040000dc):
        if constexpr (mask & 0xffff) value |= system.dma7.read_length(3);
        if constexpr (mask & 0xffff0000) value |= system.dma7.read_control(3);
        return value;
    case MMIO(0x04000180):
        return system.ipc.read_ipcsync(arm::Arch::ARMv4);
    case MMIO(0x04000184):
        return system.ipc.read_ipcfifocnt(arm::Arch::ARMv4);
    case MMIO(0x04000208):
        return system.arm7.get_irq().read_ime();
    case MMIO(0x04000210):
        return system.arm7.get_irq().read_ie();
    case MMIO(0x04000214):
        return system.arm7.get_irq().read_irf();
    case MMIO(0x04000240):
        if constexpr (mask & 0xff) value |= system.video_unit.vram.read_vramstat();
        if constexpr (mask & 0xff00) value |= static_cast<u32>(system.read_wramcnt()) << 8;
        return value;
    case MMIO(0x04100000):
        return system.ipc.read_ipcfiforecv(arm::Arch::ARMv4);
    default:
        logger.warn("ARM7Memory: unmapped mmio %d-bit read %08x", get_access_size(mask), addr + get_access_offset(mask));
        break;
    }

    return 0;
}

// TODO: apply mask to write handlers
template <u32 mask>
void ARM7Memory::mmio_write_word(u32 addr, u32 value) {
    switch (MMIO(addr)) {
    case MMIO(0x040000d4):
        system.dma7.write_source(3, value, mask);
        break;
    case MMIO(0x040000d8):
        system.dma7.write_destination(3, value, mask);
        break;
    case MMIO(0x040000dc):
        if constexpr (mask & 0xffff) system.dma7.write_length(3, value, mask);
        if constexpr (mask & 0xffff0000) system.dma7.write_control(3, value >> 16, mask >> 16);
        break;
    case MMIO(0x04000134): 
        if constexpr (mask & 0xffff) rcnt = value;
        break;
    case MMIO(0x04000180):
        if constexpr (mask & 0xffff) system.ipc.write_ipcsync(arm::Arch::ARMv4, value);
        break;
    case MMIO(0x04000184):
        if constexpr (mask & 0xffff) system.ipc.write_ipcfifocnt(arm::Arch::ARMv4, value);
        break;
    case MMIO(0x04000188):
        system.ipc.write_ipcfifosend(arm::Arch::ARMv4, value);
        break;
    case MMIO(0x04000208):
        system.arm7.get_irq().write_ime(value);
        break;
    case MMIO(0x04000210):
        system.arm7.get_irq().write_ie(value);
        break;
    case MMIO(0x04000214):
        system.arm7.get_irq().write_irf(value);
        break;
    case MMIO(0x04000300):
        if constexpr (mask & 0xff) postflg = value & 0x1;
        if constexpr (mask & 0xff00) system.write_haltcnt(value >> 8);
        break;
    case MMIO(0x04000400) ... MMIO(0x040004fc):
        system.spu.write_channel(addr, value, mask);
    case MMIO(0x04000504):
        system.spu.write_soundbias(value, mask);
        break;
    default:
        logger.warn("ARM7Memory: unmapped mmio %d-bit write %08x = %08x", get_access_size(mask), addr + get_access_offset(mask), (value & mask) >> (get_access_offset(mask) * 8));
        break;
    }
}

int ARM7Memory::get_access_size(u32 mask) {
    int size = 0;
    for (int i = 0; i < 4; i++) {
        if (mask & 0xff) {
            size += 8;
        }

        mask >>= 8;
    }

    return size;
}

u32 ARM7Memory::get_access_offset(u32 mask) {
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