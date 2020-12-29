#include <emulator/emulator.h>
#include <emulator/core/gpu.h>
#include <emulator/common/log.h>
#include <emulator/common/arithmetic.h>
#include <emulator/common/mem_helpers.h>

GPU::GPU(Emulator *emulator) : emulator(emulator), engine_a(this, 1), engine_b(this, 0) {

}

const u32* GPU::get_framebuffer(int screen) {
    switch (screen) {
    case top_screen:
        return (get_bit(15, powcnt1) ? engine_a.get_framebuffer() : engine_b.get_framebuffer());
    case bottom_screen:
        return (get_bit(15, powcnt1) ? engine_b.get_framebuffer() : engine_a.get_framebuffer());
    }
}

void GPU::render_scanline(int line) {
    if (line < 192) {
        engine_a.render_scanline(line);
        engine_b.render_scanline(line);
    }
}

bool GPU::get_vram_bank_enabled(u8 vramcnt) {
    return (get_bit(7, vramcnt) != 0);
}

u8 GPU::get_vram_bank_mst(u8 vramcnt) {
    return vramcnt & 0x7;
}

u8 GPU::get_vram_bank_offset(u8 vramcnt) {
    return (vramcnt >> 3) & 0x3;
}

template void GPU::write_lcdc(u32 addr, u16 data);
template void GPU::write_lcdc(u32 addr, u32 data);
template <typename T>
void GPU::write_lcdc(u32 addr, T data) {
    u8 *data_addr = nullptr;
    if (in_range(0x06800000, 0x0681FFFF, addr) && get_vram_bank_mst(vramcnt_a) == 0) {
        if (get_vram_bank_enabled(vramcnt_a)) {
            data_addr = &vram_a[addr & 0x1FFFF];
        }
    } else if (in_range(0x06820000, 0x0683FFFF, addr) && get_vram_bank_mst(vramcnt_b) == 0) {
        if (get_vram_bank_enabled(vramcnt_b)) {
            data_addr = &vram_b[addr - 0x1FFFF];
        }
    } else {
        log_fatal("address not implemented! 0x%04x", addr);
    }
    // implement other banks later
    for (u32 i = 0; i < sizeof(T); i++) {
        data_addr[i] = (data >> (i * 8));
    }
}

template u16 GPU::read_lcdc(u32 addr);
template u32 GPU::read_lcdc(u32 addr);
template <typename T>
T GPU::read_lcdc(u32 addr) {
    u8 *data_addr = nullptr;
    if (in_range(0x06800000, 0x0681FFFF, addr) && get_vram_bank_mst(vramcnt_a) == 0) {
        if (get_vram_bank_enabled(vramcnt_a)) {
            data_addr = &vram_a[addr & 0x1FFFF];
        }
    } else {
        log_fatal("address not implemented! 0x%04x", addr);
    }

    T return_value = 0;
    for (u32 i = 0; i < sizeof(T); i++) {
        return_value |= (data_addr[i] << (i * 8));
    }

    return return_value;

}