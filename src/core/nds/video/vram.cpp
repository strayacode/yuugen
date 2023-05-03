#include "common/logger.h"
#include "common/bits.h"
#include "core/nds/video/vram.h"

namespace core::nds {

void VRAM::reset() {
    vramstat = 0;
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

    if (is_bank_enabled(vramcnt[0])) {
        u8 ofs = get_bank_offset(vramcnt[0]);
        switch (get_bank_mst(vramcnt[0])) {
        case 0:
            lcdc.map(bank_a.data(), 0, 0x20000);
            break;
        case 1:
            bga.map(bank_a.data(), ofs * 0x20000, 0x20000);
            break;
        case 2:
            obja.map(bank_a.data(), common::get_bit<0>(ofs) * 0x20000, 0x20000);
            break;
        case 3:
            texture_data.map(bank_a.data(), ofs * 0x20000, 0x20000);
            break;
        }
    }

    if (is_bank_enabled(vramcnt[1])) {
        u8 ofs = get_bank_offset(vramcnt[1]);
        switch (get_bank_mst(vramcnt[1])) {
        case 0:
            lcdc.map(bank_b.data(), 0x20000, 0x20000);
            break;
        case 1:
            bga.map(bank_b.data(), ofs * 0x20000, 0x20000);
            break;
        case 2:
            obja.map(bank_b.data(), common::get_bit<0>(ofs) * 0x20000, 0x20000);
            break;
        case 3:
            texture_data.map(bank_b.data(), ofs * 0x20000, 0x20000);
            break;
        }
    }
    
    if (is_bank_enabled(vramcnt[2])) {
        u8 ofs = get_bank_offset(vramcnt[2]);
        switch (get_bank_mst(vramcnt[2])) {
        case 0:
            lcdc.map(bank_c.data(), 0x40000, 0x20000);
            break;
        case 1:
            bga.map(bank_c.data(), ofs * 0x20000, 0x20000);
            break;
        case 2:
            arm7_vram.map(bank_c.data(), common::get_bit<0>(ofs) * 0x20000, 0x20000);
            break;
        case 3:
            texture_data.map(bank_c.data(), ofs * 0x20000, 0x20000);
            break;
        case 4:
            bgb.map(bank_c.data(), 0, 0x20000);
            break;
        }
    }

    if (is_bank_enabled(vramcnt[2]) && (get_bank_mst(vramcnt[2]) == 2)) {
        vramstat |= 1;
    } else {
        vramstat &= ~1;
    }
    
    if (is_bank_enabled(vramcnt[3])) {
        u8 ofs = get_bank_offset(vramcnt[3]);
        switch (get_bank_mst(vramcnt[3])) {
        case 0:
            lcdc.map(bank_d.data(), 0x60000, 0x20000);
            break;
        case 1:
            bga.map(bank_d.data(), ofs * 0x20000, 0x20000);
            break;
        case 2:
            arm7_vram.map(bank_d.data(), common::get_bit<0>(ofs) * 0x20000, 0x20000);
            break;
        case 3:
            texture_data.map(bank_d.data(), ofs * 0x20000, 0x20000);
            break;
        case 4:
            objb.map(bank_d.data(), 0, 0x20000);
            break;
        }
    }

    if (is_bank_enabled(vramcnt[3]) && (get_bank_mst(vramcnt[3]) == 2)) {
        vramstat |= 1 << 1;
    } else {
        vramstat &= ~(1 << 1);
    }
    
    if (is_bank_enabled(vramcnt[4])) {
        switch (get_bank_mst(vramcnt[4])) {
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
            logger.warn("VRAM: handle mapping bank e to bg extended palette");
            break;
        }
    }

    if (is_bank_enabled(vramcnt[5])) {
        u8 ofs = get_bank_offset(vramcnt[5]);
        switch (get_bank_mst(vramcnt[5])) {
        case 0:
            lcdc.map(bank_f.data(), 0x90000, 0x4000);
            break;
        case 1:
            bga.map(bank_f.data(), common::get_bit<0>(ofs) * 0x4000 + common::get_bit<1>(ofs) * 0x10000, 0x4000);
            break;
        case 2:
            obja.map(bank_f.data(), common::get_bit<0>(ofs) * 0x4000 + common::get_bit<1>(ofs) * 0x10000, 0x4000);
            break;
        case 3:
            texture_palette.map(bank_f.data(), (common::get_bit<0>(ofs) + common::get_bit<1>(ofs) * 4) * 0x4000, 0x4000);
            break;
        case 4:
            logger.warn("VRAM: handle mapping bank f to bg extended palette");
            break;
        case 5:
            logger.warn("VRAM: handle mapping bank f to obj extended palette");
            break;
        }
    }

    if (is_bank_enabled(vramcnt[6])) {
        u8 ofs = get_bank_offset(vramcnt[6]);
        switch (get_bank_mst(vramcnt[6])) {
        case 0:
            lcdc.map(bank_g.data(), 0x94000, 0x4000);
            break;
        case 1:
            bga.map(bank_g.data(), common::get_bit<0>(ofs) * 0x4000 + common::get_bit<1>(ofs) * 0x10000, 0x4000);
            break;
        case 2:
            obja.map(bank_g.data(), common::get_bit<0>(ofs) * 0x4000 + common::get_bit<1>(ofs) * 0x10000, 0x4000);
            break;
        case 3:
            texture_palette.map(bank_g.data(), (common::get_bit<0>(ofs) + common::get_bit<1>(ofs) * 4) * 0x4000, 0x4000);
            break;
        case 4:
            logger.warn("VRAM: handle mapping bank g to bg extended palette");
            break;
        case 5:
            logger.warn("VRAM: handle mapping bank g to obj extended palette");
            break;
        }
    }

    if (is_bank_enabled(vramcnt[7])) {
        switch (get_bank_mst(vramcnt[7])) {
        case 0:
            lcdc.map(bank_h.data(), 0x98000, 0x8000);
            break;
        case 1:
            bgb.map(bank_h.data(), 0, 0x8000);
            break;
        case 2:
            logger.warn("VRAM: handle mapping bank h to bg extended palette");
            break;
        }
    }

    if (is_bank_enabled(vramcnt[8])) {
        switch (get_bank_mst(vramcnt[8])) {
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
            logger.warn("VRAM: handle mapping bank i to obj extended palette");
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
}

bool VRAM::is_bank_enabled(u8 vramcnt) {
    return common::get_bit<7>(vramcnt);
}

int VRAM::get_bank_offset(u8 vramcnt) {
    return common::get_field<3, 2>(vramcnt);
}

int VRAM::get_bank_mst(u8 vramcnt) {
    return common::get_field<0, 3>(vramcnt);
}

} // namespace core::nds