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
    case MMIO(0x04000000):
        return system.video_unit.ppu_a.read_dispcnt();
    case MMIO(0x04000004):
        if constexpr (mask & 0xffff) value |= system.video_unit.read_dispstat(arm::Arch::ARMv5);
        if constexpr (mask & 0xffff0000) value |= system.video_unit.read_vcount() << 16;
        return value;
    case MMIO(0x04000008):
        if constexpr (mask & 0xffff) value |= system.video_unit.ppu_a.read_bgcnt(0);
        if constexpr (mask & 0xffff0000) value |= system.video_unit.ppu_a.read_bgcnt(1) << 16;
        return value;
    case MMIO(0x0400000c):
        if constexpr (mask & 0xffff) value |= system.video_unit.ppu_a.read_bgcnt(2);
        if constexpr (mask & 0xffff0000) value |= system.video_unit.ppu_a.read_bgcnt(3) << 16;
        return value;
    case MMIO(0x04000048):
        if constexpr (mask & 0xffff) value |= system.video_unit.ppu_a.read_winin();
        if constexpr (mask & 0xffff0000) value |= system.video_unit.ppu_a.read_winout() << 16;
        return value;
    case MMIO(0x040000b0):
        return system.dma9.read_source(0);
    case MMIO(0x040000b8):
        if constexpr (mask & 0xffff) value |= system.dma9.read_length(0);
        if constexpr (mask & 0xffff0000) value |= system.dma9.read_control(0) << 16;
        return value;
    case MMIO(0x040000c4):
        if constexpr (mask & 0xffff) value |= system.dma9.read_length(1);
        if constexpr (mask & 0xffff0000) value |= system.dma9.read_control(1) << 16;
        return value;
    case MMIO(0x040000d0):
        if constexpr (mask & 0xffff) value |= system.dma9.read_length(2);
        if constexpr (mask & 0xffff0000) value |= system.dma9.read_control(2) << 16;
        return value;
    case MMIO(0x040000dc):
        if constexpr (mask & 0xffff) value |= system.dma9.read_length(3);
        if constexpr (mask & 0xffff0000) value |= system.dma9.read_control(3) << 16;
        return value;
    case MMIO(0x040000e0) ... MMIO(0x040000ec):
        return system.dma9.read_dmafill(addr);
    case MMIO(0x04000100):
        if constexpr (mask & 0xffff) value |= system.timers9.read_length(0);
        if constexpr (mask & 0xffff0000) value |= system.timers9.read_control(0) << 16;
        return value;
    case MMIO(0x04000104):
        if constexpr (mask & 0xffff) value |= system.timers9.read_length(1);
        if constexpr (mask & 0xffff0000) value |= system.timers9.read_control(1) << 16;
        return value;
    case MMIO(0x04000108):
        if constexpr (mask & 0xffff) value |= system.timers9.read_length(2);
        if constexpr (mask & 0xffff0000) value |= system.timers9.read_control(2) << 16;
        return value;
    case MMIO(0x0400010c):
        if constexpr (mask & 0xffff) value |= system.timers9.read_length(3);
        if constexpr (mask & 0xffff0000) value |= system.timers9.read_control(3) << 16;
        return value;
    case MMIO(0x04000130):
        if constexpr (mask & 0xffff) value |= system.input.read_keyinput();
        if constexpr (mask & 0xffff0000) logger.error("ARM9Memory: handle keycnt read");
        return value;
    case MMIO(0x04000180):
        return system.ipc.read_ipcsync(arm::Arch::ARMv5);
    case MMIO(0x04000184):
        return system.ipc.read_ipcfifocnt(arm::Arch::ARMv5);
    case MMIO(0x040001a4):
        return system.cartridge.read_romctrl();
    case MMIO(0x04000204):
        return system.read_exmemcnt();
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
    case MMIO(0x04000290):
        return system.maths_unit.read_div_numer();
    case MMIO(0x04000294):
        return system.maths_unit.read_div_numer() >> 32;
    case MMIO(0x04000298):
        return system.maths_unit.read_div_denom();
    case MMIO(0x0400029c):
        return system.maths_unit.read_div_denom() >> 32;
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
    case MMIO(0x040002b8):
        return system.maths_unit.read_sqrt_param();
    case MMIO(0x040002bc):
        return system.maths_unit.read_sqrt_param() >> 32;
    case MMIO(0x04000300):
        if constexpr (mask & 0xff) value |= read_postflg();
        return value;
    case MMIO(0x04000304):
        return system.video_unit.read_powcnt1();
    case MMIO(0x04001000):
        return system.video_unit.ppu_b.read_dispcnt();
    case MMIO(0x04001008):
        if constexpr (mask & 0xffff) value |= system.video_unit.ppu_b.read_bgcnt(0);
        if constexpr (mask & 0xffff0000) value |= system.video_unit.ppu_b.read_bgcnt(1) << 16;
        return value;
    case MMIO(0x0400100c):
        if constexpr (mask & 0xffff) value |= system.video_unit.ppu_b.read_bgcnt(2);
        if constexpr (mask & 0xffff0000) value |= system.video_unit.ppu_b.read_bgcnt(3) << 16;
        return value;
    case MMIO(0x04001048):
        if constexpr (mask & 0xffff) value |= system.video_unit.ppu_b.read_winin();
        if constexpr (mask & 0xffff0000) value |= system.video_unit.ppu_b.read_winout() << 16;
        return value;
    case MMIO(0x04004000):
    case MMIO(0x04004008):
        // dsi registers
        return 0;
    case MMIO(0x04100000):
        return system.ipc.read_ipcfiforecv(arm::Arch::ARMv5);
    case MMIO(0x04100010):
        return system.cartridge.read_data();
    default:
        logger.warn("ARM9Memory: unmapped %d-bit read %08x", get_access_size(mask), addr + get_access_offset(mask));
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
    case MMIO(0x04000004):
        if constexpr (mask & 0xffff) system.video_unit.write_dispstat(arm::Arch::ARMv5, value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.write_vcount(value >> 16, mask >> 16);
        break;
    case MMIO(0x04000008):
        if constexpr (mask & 0xffff) system.video_unit.ppu_a.write_bgcnt(0, value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_a.write_bgcnt(1, value >> 16, mask >> 16);
        break;
    case MMIO(0x0400000c):
        if constexpr (mask & 0xffff) system.video_unit.ppu_a.write_bgcnt(2, value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_a.write_bgcnt(3, value >> 16, mask >> 16);
        break;
    case MMIO(0x04000010):
        if constexpr (mask & 0xffff) system.video_unit.ppu_a.write_bghofs(0, value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_a.write_bgvofs(0, value >> 16, mask >> 16);
        break;
    case MMIO(0x04000014):
        if constexpr (mask & 0xffff) system.video_unit.ppu_a.write_bghofs(1, value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_a.write_bgvofs(1, value >> 16, mask >> 16);
        break;
    case MMIO(0x04000018):
        if constexpr (mask & 0xffff) system.video_unit.ppu_a.write_bghofs(2, value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_a.write_bgvofs(2, value >> 16, mask >> 16);
        break;
    case MMIO(0x0400001c):
        if constexpr (mask & 0xffff) system.video_unit.ppu_a.write_bghofs(3, value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_a.write_bgvofs(3, value >> 16, mask >> 16);
        break;
    case MMIO(0x04000020):
        if constexpr (mask & 0xffff) system.video_unit.ppu_a.write_bgpa(0, value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_a.write_bgpb(0, value >> 16, mask >> 16);
        break;
    case MMIO(0x04000024):
        if constexpr (mask & 0xffff) system.video_unit.ppu_a.write_bgpc(0, value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_a.write_bgpd(0, value >> 16, mask >> 16);
        break;
    case MMIO(0x04000028):
        system.video_unit.ppu_a.write_bgx(0, value, mask);
        break;
    case MMIO(0x0400002c):
        system.video_unit.ppu_a.write_bgy(0, value, mask);
        break;
    case MMIO(0x04000030):
        if constexpr (mask & 0xffff) system.video_unit.ppu_a.write_bgpa(1, value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_a.write_bgpb(1, value >> 16, mask >> 16);
        break;
    case MMIO(0x04000034):
        if constexpr (mask & 0xffff) system.video_unit.ppu_a.write_bgpc(1, value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_a.write_bgpd(1, value >> 16, mask >> 16);
        break;
    case MMIO(0x04000038):
        system.video_unit.ppu_a.write_bgx(1, value, mask);
        break;
    case MMIO(0x0400003c):
        system.video_unit.ppu_a.write_bgy(1, value, mask);
        break;
    case MMIO(0x04000040):
        if constexpr (mask & 0xffff) system.video_unit.ppu_a.write_winh(0, value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_a.write_winh(1, value >> 16, mask >> 16);
        break;
    case MMIO(0x04000044):
        if constexpr (mask & 0xffff) system.video_unit.ppu_a.write_winv(0, value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_a.write_winv(1, value >> 16, mask >> 16);
        break;
    case MMIO(0x04000048):
        if constexpr (mask & 0xffff) system.video_unit.ppu_a.write_winin(value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_a.write_winout(value >> 16, mask >> 16);
        break;
    case MMIO(0x0400004c):
        system.video_unit.ppu_a.write_mosaic(value, mask & 0xffff);
        break;
    case MMIO(0x04000050):
        if constexpr (mask & 0xffff) system.video_unit.ppu_a.write_bldcnt(value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_a.write_bldalpha(value >> 16, mask >> 16);
        break;
    case MMIO(0x04000054):
        system.video_unit.ppu_a.write_bldy(value, mask & 0xffff);
        break;
    case MMIO(0x04000058):
    case MMIO(0x0400005c):
        break;
    case MMIO(0x04000060):
        system.video_unit.gpu.write_disp3dcnt(value, mask);
        break;
    case MMIO(0x04000064):
        system.video_unit.write_dispcapcnt(value, mask);
        break;
    case MMIO(0x0400006c):
        system.video_unit.ppu_a.write_master_bright(value, mask);
        break;
    case MMIO(0x040000b0):
        system.dma9.write_source(0, value, mask);
        break;
    case MMIO(0x040000b4):
        system.dma9.write_destination(0, value, mask);
        break;
    case MMIO(0x040000b8):
        if constexpr (mask & 0xffff) system.dma9.write_length(0, value, mask);
        if constexpr (mask & 0xffff0000) system.dma9.write_control(0, value >> 16, mask >> 16);
        break;
    case MMIO(0x040000bc):
        system.dma9.write_source(1, value, mask);
        break;
    case MMIO(0x040000c0):
        system.dma9.write_destination(1, value, mask);
        break;
    case MMIO(0x040000c4):
        if constexpr (mask & 0xffff) system.dma9.write_length(1, value, mask);
        if constexpr (mask & 0xffff0000) system.dma9.write_control(1, value >> 16, mask >> 16);
        break;
    case MMIO(0x040000c8):
        system.dma9.write_source(2, value, mask);
        break;
    case MMIO(0x040000cc):
        system.dma9.write_destination(2, value, mask);
        break;
    case MMIO(0x040000d0):
        if constexpr (mask & 0xffff) system.dma9.write_length(2, value, mask);
        if constexpr (mask & 0xffff0000) system.dma9.write_control(2, value >> 16, mask >> 16);
        break;
    case MMIO(0x040000d4):
        system.dma9.write_source(3, value, mask);
        break;
    case MMIO(0x040000d8):
        system.dma9.write_destination(3, value, mask);
        break;
    case MMIO(0x040000dc):
        if constexpr (mask & 0xffff) system.dma9.write_length(3, value, mask);
        if constexpr (mask & 0xffff0000) system.dma9.write_control(3, value >> 16, mask >> 16);
        break;
    case MMIO(0x040000e0) ... MMIO(0x040000ec):
        system.dma9.write_dmafill(addr, value);
        break;
    case MMIO(0x04000100):
        if constexpr (mask & 0xffff) system.timers9.write_length(0, value, mask);
        if constexpr (mask & 0xffff0000) system.timers9.write_control(0, value >> 16, mask >> 16);
        break;
    case MMIO(0x04000104):
        if constexpr (mask & 0xffff) system.timers9.write_length(1, value, mask);
        if constexpr (mask & 0xffff0000) system.timers9.write_control(1, value >> 16, mask >> 16);
        break;
    case MMIO(0x04000108):
        if constexpr (mask & 0xffff) system.timers9.write_length(2, value, mask);
        if constexpr (mask & 0xffff0000) system.timers9.write_control(2, value >> 16, mask >> 16);
        break;
    case MMIO(0x0400010c):
        if constexpr (mask & 0xffff) system.timers9.write_length(3, value, mask);
        if constexpr (mask & 0xffff0000) system.timers9.write_control(3, value >> 16, mask >> 16);
        break;
    case MMIO(0x04000180):
        if constexpr (mask & 0xffff) system.ipc.write_ipcsync(arm::Arch::ARMv5, value, mask);
        break;
    case MMIO(0x04000184):
        if constexpr (mask & 0xffff) system.ipc.write_ipcfifocnt(arm::Arch::ARMv5, value, mask);
        break;
    case MMIO(0x04000188):
        system.ipc.write_ipcfifosend(arm::Arch::ARMv5, value);
        break;
    case MMIO(0x040001a0):
        if constexpr (mask & 0xffff) system.cartridge.write_auxspicnt(value, mask);
        if constexpr (mask & 0x00ff0000) logger.error("handle auxspidata writes");
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
    case MMIO(0x04000204):
        if constexpr (mask & 0xffff) system.write_exmemcnt(value, mask);
        break;
    case MMIO(0x04000208):
        system.arm9.get_irq().write_ime(value, mask);
        break;
    case MMIO(0x04000210):
        system.arm9.get_irq().write_ie(value, mask);
        break;
    case MMIO(0x04000214):
        system.arm9.get_irq().write_irf(value, mask);
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
        if constexpr (mask & 0xff) write_postflg(value);
        break;
    case MMIO(0x04000304):
        system.video_unit.write_powcnt1(value, mask);
        break;
    case MMIO(0x04001000):
        system.video_unit.ppu_b.write_dispcnt(value, mask);
        break;
    case MMIO(0x04001008):
        if constexpr (mask & 0xffff) system.video_unit.ppu_b.write_bgcnt(0, value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_b.write_bgcnt(1, value >> 16, mask >> 16);
        break;
    case MMIO(0x0400100c):
        if constexpr (mask & 0xffff) system.video_unit.ppu_b.write_bgcnt(2, value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_b.write_bgcnt(3, value >> 16, mask >> 16);
        break;
    case MMIO(0x04001010):
        if constexpr (mask & 0xffff) system.video_unit.ppu_b.write_bghofs(0, value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_b.write_bgvofs(0, value >> 16, mask >> 16);
        break;
    case MMIO(0x04001014):
        if constexpr (mask & 0xffff) system.video_unit.ppu_b.write_bghofs(1, value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_b.write_bgvofs(1, value >> 16, mask >> 16);
        break;
    case MMIO(0x04001018):
        if constexpr (mask & 0xffff) system.video_unit.ppu_b.write_bghofs(2, value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_b.write_bgvofs(2, value >> 16, mask >> 16);
        break;
    case MMIO(0x0400101c):
        if constexpr (mask & 0xffff) system.video_unit.ppu_b.write_bghofs(3, value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_b.write_bgvofs(3, value >> 16, mask >> 16);
        break;
    case MMIO(0x04001020):
        if constexpr (mask & 0xffff) system.video_unit.ppu_b.write_bgpa(0, value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_b.write_bgpb(0, value >> 16, mask >> 16);
        break;
    case MMIO(0x04001024):
        if constexpr (mask & 0xffff) system.video_unit.ppu_b.write_bgpc(0, value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_b.write_bgpd(0, value >> 16, mask >> 16);
        break;
    case MMIO(0x04001028):
        system.video_unit.ppu_b.write_bgx(0, value, mask);
        break;
    case MMIO(0x0400102c):
        system.video_unit.ppu_b.write_bgy(0, value, mask);
        break;
    case MMIO(0x04001030):
        if constexpr (mask & 0xffff) system.video_unit.ppu_b.write_bgpa(1, value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_b.write_bgpb(1, value >> 16, mask >> 16);
        break;
    case MMIO(0x04001034):
        if constexpr (mask & 0xffff) system.video_unit.ppu_b.write_bgpc(1, value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_b.write_bgpd(1, value >> 16, mask >> 16);
        break;
    case MMIO(0x04001038):
        system.video_unit.ppu_b.write_bgx(1, value, mask);
        break;
    case MMIO(0x0400103c):
        system.video_unit.ppu_b.write_bgy(1, value, mask);
        break;
    case MMIO(0x04001040):
        if constexpr (mask & 0xffff) system.video_unit.ppu_b.write_winh(0, value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_b.write_winh(1, value >> 16, mask >> 16);
        break;
    case MMIO(0x04001044):
        if constexpr (mask & 0xffff) system.video_unit.ppu_b.write_winv(0, value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_b.write_winv(1, value >> 16, mask >> 16);
        break;
    case MMIO(0x04001048):
        if constexpr (mask & 0xffff) system.video_unit.ppu_b.write_winin(value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_b.write_winout(value >> 16, mask >> 16);
        break;
    case MMIO(0x0400104c):
        system.video_unit.ppu_b.write_mosaic(value, mask & 0xffff);
        break;
    case MMIO(0x04001050):
        if constexpr (mask & 0xffff) system.video_unit.ppu_b.write_bldcnt(value, mask);
        if constexpr (mask & 0xffff0000) system.video_unit.ppu_b.write_bldalpha(value >> 16, mask >> 16);
        break;
    case MMIO(0x04001054):
        system.video_unit.ppu_b.write_bldy(value, mask & 0xffff);
        break;
    case MMIO(0x04001004):
    case MMIO(0x04001058):
    case MMIO(0x0400105c):
    case MMIO(0x04001060):
    case MMIO(0x04001064):
    case MMIO(0x04001068):
        break;
    case MMIO(0x0400106c):
        system.video_unit.ppu_b.write_master_bright(value, mask);
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