#include <emulator/Emulator.h>
#include <emulator/core/GPU.h>
#include <emulator/common/log.h>
#include <emulator/common/arithmetic.h>
#include <emulator/common/mem_helpers.h>
#include <string.h>

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
    // set hblank flag
    dispstat |= (1 << 1);

    vcount++;
    // printf("vcount %d dispstat thing %d", vcount, dispstat >> 8);
    switch (vcount) {
    case 160:
        // set vblank flag in dispstat
        dispstat |= 1;
        break;
    case 227:
        // clear vblank flag in dispstat at end of blank 
        dispstat &= ~1;
        break;
    case 228:
        // reset vcount
        vcount = 0;

        // clear hblank flag as hblank flag is set for lines 0..227
        dispstat &= ~(1 << 1);
        break;
    }
    if (line < 192) {
        engine_a.render_scanline(line);
        engine_b.render_scanline(line);
    }
    
    // check if vcount is equal to lyc
    if (vcount == (dispstat >> 8)) {
        // set the v counter flag
        dispstat |= (1 << 2);
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
    // log_debug("lcdc write to addr: 0x%04x and data 0x%04x", addr, data);

    if (in_range(0x06800000, 0x0681FFFF, addr) && get_vram_bank_mst(vramcnt_a) == 0) {
        if (get_vram_bank_enabled(vramcnt_a)) {
            memcpy(&vram_a[addr & 0x1FFFF], &data, sizeof(T));
        }
    } else if (in_range(0x06820000, 0x0683FFFF, addr) && get_vram_bank_mst(vramcnt_b) == 0) {
        if (get_vram_bank_enabled(vramcnt_b)) {
            memcpy(&vram_b[addr & 0x1FFFF], &data, sizeof(T));
        }
    } else if (in_range(0x06840000, 0x0685FFFF, addr) && get_vram_bank_mst(vramcnt_c) == 0) {
        if (get_vram_bank_enabled(vramcnt_c)) {
            memcpy(&vram_c[addr & 0x1FFFF], &data, sizeof(T));
        }
    } else if (in_range(0x06860000, 0x0687FFFF, addr) && get_vram_bank_mst(vramcnt_d) == 0) {
        if (get_vram_bank_enabled(vramcnt_d)) {
            memcpy(&vram_d[addr & 0x1FFFF], &data, sizeof(T));
        }
    } else if (in_range(0x06880000, 0x0688FFFF, addr) && get_vram_bank_mst(vramcnt_e) == 0) {
        if (get_vram_bank_enabled(vramcnt_e)) {
            memcpy(&vram_e[addr & 0xFFFF], &data, sizeof(T));
        }
    } else if (in_range(0x06890000, 0x06893FFF, addr) && get_vram_bank_mst(vramcnt_f) == 0) {
        if (get_vram_bank_enabled(vramcnt_f)) {
            memcpy(&vram_f[addr & 0x3FFF], &data, sizeof(T));
        }
    } else if (in_range(0x06894000, 0x06897FFF, addr) && get_vram_bank_mst(vramcnt_g) == 0) {
        if (get_vram_bank_enabled(vramcnt_g)) {
            memcpy(&vram_g[addr & 0x3FFF], &data, sizeof(T));
        }
    } else if (in_range(0x06898000, 0x0689FFFF, addr) && get_vram_bank_mst(vramcnt_h) == 0) {
        if (get_vram_bank_enabled(vramcnt_h)) {
            memcpy(&vram_h[addr & 0x7FFF], &data, sizeof(T));
        }
    } else if (in_range(0x068A0000, 0x068A3FFF, addr) && get_vram_bank_mst(vramcnt_i) == 0) {
        if (get_vram_bank_enabled(vramcnt_i)) {
            memcpy(&vram_i[addr & 0x3FFF], &data, sizeof(T));
        }
    } 
}

template u16 GPU::read_lcdc(u32 addr);
template u32 GPU::read_lcdc(u32 addr);
template <typename T>
T GPU::read_lcdc(u32 addr) {
    T return_value = 0;
    if (in_range(0x06800000, 0x0681FFFF, addr) && get_vram_bank_mst(vramcnt_a) == 0) {
        if (get_vram_bank_enabled(vramcnt_a)) {
            memcpy(&return_value, &vram_a[addr & 0x1FFFF], sizeof(T));
        }
    } else {
        log_fatal("address not implemented! 0x%04x", addr);
    }

    return return_value;

}