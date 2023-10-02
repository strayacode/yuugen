#include "common/logger.h"
#include "common/bits.h"
#include "nds/video/vram.h"

namespace nds {

void VRAM::reset() {
    vramstat = 0;
    vramcnt.fill(VRAMCNT{});
    bank_a.fill(0);
    bank_b.fill(0);
    bank_c.fill(0);
    bank_d.fill(0);
    bank_e.fill(0);
    bank_f.fill(0);
    bank_g.fill(0);
    bank_h.fill(0);
    bank_i.fill(0);

    lcdc.allocate(0xa4000);
    bga.allocate(0x80000);
    obja.allocate(0x40000);
    bgb.allocate(0x20000);
    objb.allocate(0x20000);
    arm7_vram.allocate(0x20000);
    texture_data.allocate(0x80000);
    texture_palette.allocate(0x20000);
    bga_extended_palette.allocate(0x8000);
    bgb_extended_palette.allocate(0x8000);
    obja_extended_palette.allocate(0x2000);
    objb_extended_palette.allocate(0x2000);

    reset_vram_regions();
}

void VRAM::write_vramcnt(Bank bank, u8 value) {
    const u8 masks[] = {0x9b, 0x9b, 0x9f, 0x9f, 0x87, 0x9f, 0x9f, 0x83, 0x83};
    int index = static_cast<int>(bank);
    value &= masks[index];

    if (vramcnt[index].data == value) {
        return;
    }

    vramcnt[index].data = value;
    reset_vram_regions();

    if (vramcnt[0].enable) {
        auto offset = vramcnt[0].offset;
        switch (vramcnt[0].mst) {
        case 0:
            lcdc.map(bank_a.data(), 0, 0x20000);
            break;
        case 1:
            bga.map(bank_a.data(), offset * 0x20000, 0x20000);
            break;
        case 2:
            obja.map(bank_a.data(), common::get_bit<0>(offset) * 0x20000, 0x20000);
            break;
        case 3:
            texture_data.map(bank_a.data(), offset * 0x20000, 0x20000);
            break;
        }
    }

    if (vramcnt[1].enable) {
        auto offset = vramcnt[1].offset;
        switch (vramcnt[1].mst) {
        case 0:
            lcdc.map(bank_b.data(), 0x20000, 0x20000);
            break;
        case 1:
            bga.map(bank_b.data(), offset * 0x20000, 0x20000);
            break;
        case 2:
            obja.map(bank_b.data(), common::get_bit<0>(offset) * 0x20000, 0x20000);
            break;
        case 3:
            texture_data.map(bank_b.data(), offset * 0x20000, 0x20000);
            break;
        }
    }
    
    if (vramcnt[2].enable) {
        auto offset = vramcnt[2].offset;
        switch (vramcnt[2].mst) {
        case 0:
            lcdc.map(bank_c.data(), 0x40000, 0x20000);
            break;
        case 1:
            bga.map(bank_c.data(), offset * 0x20000, 0x20000);
            break;
        case 2:
            arm7_vram.map(bank_c.data(), common::get_bit<0>(offset) * 0x20000, 0x20000);
            break;
        case 3:
            texture_data.map(bank_c.data(), offset * 0x20000, 0x20000);
            break;
        case 4:
            bgb.map(bank_c.data(), 0, 0x20000);
            break;
        }
    }

    if (vramcnt[2].enable && (vramcnt[2].mst == 2)) {
        vramstat |= 1;
    } else {
        vramstat &= ~1;
    }
    
    if (vramcnt[3].enable) {
        auto offset = vramcnt[3].offset;
        switch (vramcnt[3].mst) {
        case 0:
            lcdc.map(bank_d.data(), 0x60000, 0x20000);
            break;
        case 1:
            bga.map(bank_d.data(), offset * 0x20000, 0x20000);
            break;
        case 2:
            arm7_vram.map(bank_d.data(), common::get_bit<0>(offset) * 0x20000, 0x20000);
            break;
        case 3:
            texture_data.map(bank_d.data(), offset * 0x20000, 0x20000);
            break;
        case 4:
            objb.map(bank_d.data(), 0, 0x20000);
            break;
        }
    }

    if (vramcnt[3].enable && (vramcnt[3].mst == 2)) {
        vramstat |= 1 << 1;
    } else {
        vramstat &= ~(1 << 1);
    }
    
    if (vramcnt[4].enable) {
        switch (vramcnt[4].mst) {
        case 0:
            lcdc.map(bank_e.data(), 0x80000, 0x10000);
            break;
        case 1:
            bga.map(bank_e.data(), 0, 0x10000);
            break;
        case 2:
            obja.map(bank_e.data(), 0, 0x10000);
            break;
        case 3:
            texture_palette.map(bank_e.data(), 0, 0x10000);
            break;
        case 4:
            bga_extended_palette.map(bank_e.data(), 0, 0x8000);
            break;
        }
    }

    if (vramcnt[5].enable) {
        auto offset = vramcnt[5].offset;
        switch (vramcnt[5].mst) {
        case 0:
            lcdc.map(bank_f.data(), 0x90000, 0x4000);
            break;
        case 1:
            bga.map(bank_f.data(), common::get_bit<0>(offset) * 0x4000 + common::get_bit<1>(offset) * 0x10000, 0x4000);
            break;
        case 2:
            obja.map(bank_f.data(), common::get_bit<0>(offset) * 0x4000 + common::get_bit<1>(offset) * 0x10000, 0x4000);
            break;
        case 3:
            texture_palette.map(bank_f.data(), (common::get_bit<0>(offset) + common::get_bit<1>(offset) * 4) * 0x4000, 0x4000);
            break;
        case 4:
            bga_extended_palette.map(bank_f.data(), common::get_bit<0>(offset) * 0x4000, 0x4000);
            break;
        case 5:
            obja_extended_palette.map(bank_f.data(), 0, 0x2000);
            break;
        }
    }

    if (vramcnt[6].enable) {
        auto offset = vramcnt[6].offset;
        switch (vramcnt[6].mst) {
        case 0:
            lcdc.map(bank_g.data(), 0x94000, 0x4000);
            break;
        case 1:
            bga.map(bank_g.data(), common::get_bit<0>(offset) * 0x4000 + common::get_bit<1>(offset) * 0x10000, 0x4000);
            break;
        case 2:
            obja.map(bank_g.data(), common::get_bit<0>(offset) * 0x4000 + common::get_bit<1>(offset) * 0x10000, 0x4000);
            break;
        case 3:
            texture_palette.map(bank_g.data(), (common::get_bit<0>(offset) + common::get_bit<1>(offset) * 4) * 0x4000, 0x4000);
            break;
        case 4:
            bga_extended_palette.map(bank_g.data(), common::get_bit<0>(offset) * 0x4000, 0x4000);
            break;
        case 5:
            obja_extended_palette.map(bank_g.data(), 0, 0x2000);
            break;
        }
    }

    if (vramcnt[7].enable) {
        switch (vramcnt[7].mst) {
        case 0:
            lcdc.map(bank_h.data(), 0x98000, 0x8000);
            break;
        case 1:
            bgb.map(bank_h.data(), 0, 0x8000);
            break;
        case 2:
            bgb_extended_palette.map(bank_h.data(), 0, 0x8000);
            break;
        }
    }

    if (vramcnt[8].enable) {
        switch (vramcnt[8].mst) {
        case 0:
            lcdc.map(bank_i.data(), 0xa0000, 0x4000);
            break;
        case 1:
            bgb.map(bank_i.data(), 0x8000, 0x4000);
            break;
        case 2:
            objb.map(bank_i.data(), 0, 0x4000);
            break;
        case 3:
            objb_extended_palette.map(bank_i.data(), 0, 0x2000);
            break;
        }
    }
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
    bga_extended_palette.reset();
    bgb_extended_palette.reset();
}

} // namespace nds