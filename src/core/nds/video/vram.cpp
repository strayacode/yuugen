#include "core/nds/video/vram.h"

namespace core::nds {

VRAM::reset() {
    vramcnt.fill(0);
    bank_a.fill(0);
    bank_b.fill(0);
    bank_c.fill(0);
    bank_d.fill(0);
    bank_e.fill(0);
    bank_f.fill(0);
    bank_g.fill(0);
    bank_h.fill(0);
    bank_i.fill(0);
    reset_vram_regions();
}

void VRAM::write_vramcnt(Bank bank, u8 value) {
    const u8 masks[] = {0x9b, 0x9b, 0x9f, 0x9f, 0x87, 0x9f, 0x9f, 0x83, 0x83};
    int index = static_cast<int>(bank);
    value &= masks[index];

    if (vramcnt[index] == value) {
        return;
    }

    vramcnt[index] = value;

    reset_vram_regions();
}

void VRAM::reset_vram_regions() {
    lcdc.reset();
    bga.reset();
    obja.reset();
    bgb.reset();
    objb.reset();
    arm7_vram.reset();
    texture_data.reset();
    texture_palette.reset();
}

} // namespace core::nds