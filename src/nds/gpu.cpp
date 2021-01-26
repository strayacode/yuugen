#include <nds/gpu.h>
#include <nds/nds.h>


GPU::GPU(NDS *nds) : nds(nds), engine_a(this, 0), engine_b(this, 1) {

}

void GPU::write_dispstat9(u16 data) {
	// make sure to only change bits which are writeable
	DISPSTAT9 = (data & 0xFFB8);
}

void GPU::write_dispstat7(u16 data) {
    // make sure to only change bits which are writeable
    DISPSTAT7 = (data & 0xFFB8);
}

void GPU::render_scanline_begin(int line) {
    // set hblank flag as the gpu has now done 256 dots on the scanline and is ready to start hblank
    DISPSTAT9 |= (1 << 1);
    DISPSTAT7 |= (1 << 1);

    
    if (line < 192) {
        engine_a.render_scanline(line);
        engine_b.render_scanline(line);
    }
}

void GPU::render_scanline_end(int line) {
	VCOUNT++;
	switch (VCOUNT) {
    case 192:
        // set vblank flag in dispstat as vblank starts when 192 scanlines have been rendered
        DISPSTAT9 |= 1;
        DISPSTAT7 |= 1;
        break;
    case 262: // last scanline
        // clear vblank flag in dispstat at end of vblank 
        DISPSTAT9 &= ~1;
        DISPSTAT7 &= ~1;
        break;
    case 263: // frame is finished
        // reset vcount
        VCOUNT = 0;
        break;
    }


	// clear the hblank flag as we are now at the end of the scanline as hblank has finished
	DISPSTAT9 &= ~(1 << 1);
    DISPSTAT7 &= ~(1 << 1);

	if ((DISPSTAT9 >> 8) == VCOUNT) {
		// set the v counter flag
		DISPSTAT7 |= (1 << 2);
	} else if (get_bit(2, DISPSTAT9)) {
        // reset v counter flag on next line as at that point v count will not be the same as lyc and thus v counter flag must be reset
        DISPSTAT9 &= ~(1 << 2);
    }

    if ((DISPSTAT7 >> 8) == VCOUNT) {
        // set the v counter flag
        DISPSTAT7 |= (1 << 2);
    } else if (get_bit(2, DISPSTAT7)) {
        // reset v counter flag on next line as at that point v count will not be the same as lyc and thus v counter flag must be reset
        DISPSTAT7 &= ~(1 << 2);
    }
}

const u32* GPU::get_framebuffer(int screen) {
    switch (screen) {
    case TOP_SCREEN:
        return (get_bit(15, POWCNT1) ? engine_a.get_framebuffer() : engine_b.get_framebuffer());
    case BOTTOM_SCREEN:
        return (get_bit(15, POWCNT1) ? engine_b.get_framebuffer() : engine_a.get_framebuffer());
    }
}

void GPU::write_lcdc_vram(u32 addr, u16 data) {
	if (in_range(0x06800000, 0x0681FFFF, addr) && !get_vram_bank_mst(vramcnt_a)) {
        if (get_vram_bank_enabled(vramcnt_a)) {
            memcpy(&vram_a[addr & 0x1FFFF], &data, 2);
        }
    } else if (in_range(0x06820000, 0x0683FFFF, addr) && !get_vram_bank_mst(vramcnt_b)) {
        if (get_vram_bank_enabled(vramcnt_b)) {
            memcpy(&vram_b[addr & 0x1FFFF], &data, 2);
        }
    } else if (in_range(0x06840000, 0x0685FFFF, addr) && !get_vram_bank_mst(vramcnt_c)) {
        if (get_vram_bank_enabled(vramcnt_c)) {
            memcpy(&vram_c[addr & 0x1FFFF], &data, 2);
        }
    } else if (in_range(0x06860000, 0x0687FFFF, addr) && !get_vram_bank_mst(vramcnt_d)) {
        if (get_vram_bank_enabled(vramcnt_d)) {
            memcpy(&vram_d[addr & 0x1FFFF], &data, 2);
        }
    } else if (in_range(0x06880000, 0x0688FFFF, addr) && !get_vram_bank_mst(vramcnt_e)) {
        if (get_vram_bank_enabled(vramcnt_e)) {
            memcpy(&vram_e[addr & 0xFFFF], &data, 2);
        }
    } else if (in_range(0x06890000, 0x06893FFF, addr) && !get_vram_bank_mst(vramcnt_f)) {
        if (get_vram_bank_enabled(vramcnt_f)) {
            memcpy(&vram_f[addr & 0x3FFF], &data, 2);
        }
    } else if (in_range(0x06894000, 0x06897FFF, addr) && !get_vram_bank_mst(vramcnt_g)) {
        if (get_vram_bank_enabled(vramcnt_g)) {
            memcpy(&vram_g[addr & 0x3FFF], &data, 2);
        }
    } else if (in_range(0x06898000, 0x0689FFFF, addr) && !get_vram_bank_mst(vramcnt_h)) {
        if (get_vram_bank_enabled(vramcnt_h)) {
            memcpy(&vram_h[addr & 0x7FFF], &data, 2);
        }
    } else if (in_range(0x068A0000, 0x068A3FFF, addr) && !get_vram_bank_mst(vramcnt_i)) {
        if (get_vram_bank_enabled(vramcnt_i)) {
            memcpy(&vram_i[addr & 0x3FFF], &data, 2);
        }
    } 
}

bool GPU::get_vram_bank_enabled(u8 vramcnt) {
	return (vramcnt & (1 << 7));
}

bool GPU::get_vram_bank_mst(u8 vramcnt) {
	return vramcnt & 0x7;
}