#include "common/logger.h"
#include "arm/arch.h"
#include "nds/arm7/arm7.h"
#include "nds/system.h"

namespace nds {

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
    case MMIO(0x04000004):
        if constexpr (mask & 0xffff) value |= system.video_unit.read_dispstat(arm::Arch::ARMv4);
        if constexpr (mask & 0xffff0000) value |= system.video_unit.read_vcount() << 16;
        return value;
    case MMIO(0x040000b0):
        return system.dma7.read_source(0);
    case MMIO(0x040000b8):
        if constexpr (mask & 0xffff) value |= system.dma7.read_length(0);
        if constexpr (mask & 0xffff0000) value |= system.dma7.read_control(0) << 16;
        return value;
    case MMIO(0x040000c4):
        if constexpr (mask & 0xffff) value |= system.dma7.read_length(1);
        if constexpr (mask & 0xffff0000) value |= system.dma7.read_control(1) << 16;
        return value;
    case MMIO(0x040000d0):
        if constexpr (mask & 0xffff) value |= system.dma7.read_length(2);
        if constexpr (mask & 0xffff0000) value |= system.dma7.read_control(2) << 16;
        return value;
    case MMIO(0x040000dc):
        if constexpr (mask & 0xffff) value |= system.dma7.read_length(3);
        if constexpr (mask & 0xffff0000) value |= system.dma7.read_control(3) << 16;
        return value;
    case MMIO(0x04000100):
        if constexpr (mask & 0xffff) value |= system.timers7.read_length(0);
        if constexpr (mask & 0xffff0000) value |= system.timers7.read_control(0) << 16;
        return value;
    case MMIO(0x04000104):
        if constexpr (mask & 0xffff) value |= system.timers7.read_length(1);
        if constexpr (mask & 0xffff0000) value |= system.timers7.read_control(1) << 16;
        return value;
    case MMIO(0x04000108):
        if constexpr (mask & 0xffff) value |= system.timers7.read_length(2);
        if constexpr (mask & 0xffff0000) value |= system.timers7.read_control(2) << 16;
        return value;
    case MMIO(0x0400010c):
        if constexpr (mask & 0xffff) value |= system.timers7.read_length(3);
        if constexpr (mask & 0xffff0000) value |= system.timers7.read_control(3) << 16;
        return value;
    case MMIO(0x04000130):
        if constexpr (mask & 0xffff) value |= system.input.read_keyinput();
        if constexpr (mask & 0xffff0000) logger.error("ARM7Memory: handle keycnt read");
        return value;
    case MMIO(0x04000134):
        if constexpr (mask & 0xffff) value |= system.read_rcnt();
        if constexpr (mask & 0xffff0000) value |= system.input.read_extkeyin() << 16;
        return value;
    case MMIO(0x04000138):
        if constexpr (mask & 0xff) value |= system.rtc.read_rtc();
        return value;
    case MMIO(0x04000180):
        return system.ipc.read_ipcsync(arm::Arch::ARMv4);
    case MMIO(0x04000184):
        return system.ipc.read_ipcfifocnt(arm::Arch::ARMv4);
    case MMIO(0x040001a0):
        if constexpr (mask & 0xffff) value |= system.cartridge.read_auxspicnt();
        if constexpr (mask & 0xffff0000) value |= system.cartridge.read_auxspidata() << 16;
        return value;
    case MMIO(0x040001a4):
        return system.cartridge.read_romctrl();
    case MMIO(0x040001a8):
        return system.cartridge.read_command_buffer();
    case MMIO(0x040001ac):
        return system.cartridge.read_command_buffer() >> 32;
    case MMIO(0x040001c0):
        if constexpr (mask & 0xffff) value |= system.spi.read_spicnt();
        if constexpr (mask & 0xff0000) value |= system.spi.read_spidata() << 16;
        return value;
    case MMIO(0x04000204):
        if constexpr (mask & 0xffff) value |= system.read_exmemstat();
        if constexpr (mask & 0xffff0000) logger.error("ARM7Memory: handle wifiwaitcnt reads");
        return value;
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
    case MMIO(0x04000300):
        if constexpr (mask & 0xff) value |= read_postflg();
        return value;
    case MMIO(0x04000304):
        return system.video_unit.read_powcnt1();
    case MMIO(0x04000400) ... MMIO(0x040004fc):
        return system.spu.read_channel(addr);
    case MMIO(0x04000500):
        return system.spu.read_soundcnt();
    case MMIO(0x04000504):
        return system.spu.read_soundbias();
    case MMIO(0x04000508):
        if constexpr (mask & 0xff) value |= system.spu.read_sound_capture_control(0);
        if constexpr (mask & 0xff00) value |= system.spu.read_sound_capture_control(1) << 8;
        return value;
    case MMIO(0x04100000):
        return system.ipc.read_ipcfiforecv(arm::Arch::ARMv4);
    case MMIO(0x04100010):
        return system.cartridge.read_data();
    default:
        if (addr >= 0x04800000 && addr < 0x04900000) {
            // TODO: handle wifi reads
            return 0;
        }

        logger.warn("ARM7Memory: unmapped mmio %d-bit read %08x", get_access_size(mask), addr + get_access_offset(mask));
        break;
    }

    return 0;
}

template <u32 mask>
void ARM7Memory::mmio_write_word(u32 addr, u32 value) {
    switch (MMIO(addr)) {
    case MMIO(0x04000004):
        if constexpr (mask & 0xffff) system.video_unit.write_dispstat(arm::Arch::ARMv4, value, mask);
        if constexpr (mask & 0xffff0000) logger.error("ARM7Memory: handle vcount writes");
        break;
    case MMIO(0x040000b0):
        system.dma7.write_source(0, value, mask);
        break;
    case MMIO(0x040000b4):
        system.dma7.write_destination(0, value, mask);
        break;
    case MMIO(0x040000b8):
        if constexpr (mask & 0xffff) system.dma7.write_length(0, value, mask);
        if constexpr (mask & 0xffff0000) system.dma7.write_control(0, value >> 16, mask >> 16);
        break;
    case MMIO(0x040000bc):
        system.dma7.write_source(1, value, mask);
        break;
    case MMIO(0x040000c0):
        system.dma7.write_destination(1, value, mask);
        break;
    case MMIO(0x040000c4):
        if constexpr (mask & 0xffff) system.dma7.write_length(1, value, mask);
        if constexpr (mask & 0xffff0000) system.dma7.write_control(1, value >> 16, mask >> 16);
        break;
    case MMIO(0x040000c8):
        system.dma7.write_source(2, value, mask);
        break;
    case MMIO(0x040000cc):
        system.dma7.write_destination(2, value, mask);
        break;
    case MMIO(0x040000d0):
        if constexpr (mask & 0xffff) system.dma7.write_length(2, value, mask);
        if constexpr (mask & 0xffff0000) system.dma7.write_control(2, value >> 16, mask >> 16);
        break;
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
    case MMIO(0x04000100):
        if constexpr (mask & 0xffff) system.timers7.write_length(0, value, mask);
        if constexpr (mask & 0xffff0000) system.timers7.write_control(0, value >> 16, mask >> 16);
        break;
    case MMIO(0x04000104):
        if constexpr (mask & 0xffff) system.timers7.write_length(1, value, mask);
        if constexpr (mask & 0xffff0000) system.timers7.write_control(1, value >> 16, mask >> 16);
        break;
    case MMIO(0x04000108):
        if constexpr (mask & 0xffff) system.timers7.write_length(2, value, mask);
        if constexpr (mask & 0xffff0000) system.timers7.write_control(2, value >> 16, mask >> 16);
        break;
    case MMIO(0x0400010c):
        if constexpr (mask & 0xffff) system.timers7.write_length(3, value, mask);
        if constexpr (mask & 0xffff0000) system.timers7.write_control(3, value >> 16, mask >> 16);
        break;
    case MMIO(0x04000120):
        // siodata
        break;
    case MMIO(0x04000128):
        // siocnt
        break;
    case MMIO(0x04000134): 
        if constexpr (mask & 0xffff) rcnt = value;
        break;
    case MMIO(0x04000138):
        if constexpr (mask & 0xff) system.rtc.write_rtc(value);
        break;
    case MMIO(0x04000180):
        if constexpr (mask & 0xffff) system.ipc.write_ipcsync(arm::Arch::ARMv4, value, mask);
        break;
    case MMIO(0x04000184):
        if constexpr (mask & 0xffff) system.ipc.write_ipcfifocnt(arm::Arch::ARMv4, value, mask);
        break;
    case MMIO(0x04000188):
        system.ipc.write_ipcfifosend(arm::Arch::ARMv4, value);
        break;
    case MMIO(0x040001a0):
        if constexpr (mask & 0xffff) system.cartridge.write_auxspicnt(value, mask);
        if constexpr (mask & 0xffff0000) system.cartridge.write_auxspidata(value >> 16);
        break;
    case MMIO(0x040001a4):
        system.cartridge.write_romctrl(value, mask);
        break;
    case MMIO(0x040001a8):
        system.cartridge.write_command_buffer(value, mask);
        break;
    case MMIO(0x040001ac):
        system.cartridge.write_command_buffer(static_cast<u64>(value) << 32, static_cast<u64>(mask) << 32);
        break;
    case MMIO(0x040001b0):
    case MMIO(0x040001b4):
    case MMIO(0x040001b8):
        break;
    case MMIO(0x040001c0):
        if constexpr (mask & 0xffff) system.spi.write_spicnt(value, mask & 0xffff);
        if constexpr (mask & 0xffff0000) system.spi.write_spidata(value >> 16);
        break;
    case MMIO(0x04000204):
        if constexpr (mask & 0xffff) system.write_exmemstat(value, mask & 0xffff);
        if constexpr (mask & 0xffff0000) system.wifi.write_wifiwaitcnt(value >> 16, mask >> 16);
        break;
    case MMIO(0x04000208):
        system.arm7.get_irq().write_ime(value, mask);
        break;
    case MMIO(0x04000210):
        system.arm7.get_irq().write_ie(value, mask);
        break;
    case MMIO(0x04000214):
        system.arm7.get_irq().write_irf(value, mask);
        break;
    case MMIO(0x04000300):
        if constexpr (mask & 0xff) write_postflg(value);
        if constexpr (mask & 0xff00) system.write_haltcnt(value >> 8);
        break;
    case MMIO(0x04000304):
        system.video_unit.write_powcnt1(value, mask);
        break;
    case MMIO(0x04000308): 
        biosprot = value;
        break;
    case MMIO(0x04000400) ... MMIO(0x040004fc):
        system.spu.write_channel(addr, value, mask);
        break;
    case MMIO(0x04000500):
        system.spu.write_soundcnt(value, mask);
        break;
    case MMIO(0x04000504):
        system.spu.write_soundbias(value, mask);
        break;
    case MMIO(0x04000508):
        if constexpr (mask & 0xff) system.spu.write_sound_capture_control(0, value);
        if constexpr (mask & 0xff00) system.spu.write_sound_capture_control(1, value >> 8);
        break;
    case MMIO(0x04000510):
        system.spu.write_sound_capture_destination(0, value, mask);
        break;
    case MMIO(0x04000514):
        system.spu.write_sound_capture_length(0, value, mask);
        break;
    case MMIO(0x04000518):
        system.spu.write_sound_capture_destination(1, value, mask);
        break;
    case MMIO(0x0400051c):
        system.spu.write_sound_capture_length(1, value, mask);
        break;
    case MMIO(0x04001080):
        // sio or debugging related
        break;
    default:
        if (addr >= 0x04800000 && addr < 0x04900000) {
            // TODO: handle wifi writes
            break;
        }

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

} // namespace nds