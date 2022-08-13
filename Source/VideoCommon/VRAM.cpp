#include "VideoCommon/VRAM.h"

void VRAM::reset() {
    vramcnt.fill(0);
    vramstat = 0;
    
    bank_a.fill(0);
    bank_b.fill(0);
    bank_c.fill(0);
    bank_d.fill(0);
    bank_e.fill(0);
    bank_f.fill(0);
    bank_g.fill(0);
    bank_h.fill(0);
    bank_i.fill(0);

    reset_vram_mapping();
}

void VRAM::update_vram_mapping(Bank bank, u8 data) {
    const u8 masks[] = {0x9b, 0x9b, 0x9f, 0x9f, 0x87, 0x9f, 0x9f, 0x83, 0x83};
    int index = static_cast<int>(bank);

    data &= masks[index];

    if (vramcnt[index] == data) return;

    vramcnt[index] = data;

    // reset all the vram pages
    reset_vram_mapping();

    // remap bank a
    if (get_bank_enabled(vramcnt[0])) {
        u8 ofs = get_bank_offset(vramcnt[0]);
        switch (get_bank_mst(vramcnt[0])) {
        case 0:
            lcdc.map(bank_a.data(), 0, 32);
            break;
        case 1:
            bga.map(bank_a.data(), ofs * 32, 32);
            break;
        case 2:
            obja.map(bank_a.data(), (ofs & 0x1) * 32, 32);
            break;
        case 3:
            texture_data.map(bank_a.data(), ofs * 32, 32);
            break;
        default:
            log_fatal("handle mst %d", get_bank_mst(vramcnt[0]));
        }
    }

    // remap bank b
    if (get_bank_enabled(vramcnt[1])) {
        u8 ofs = get_bank_offset(vramcnt[1]);
        switch (get_bank_mst(vramcnt[1])) {
        case 0:
            lcdc.map(bank_b.data(), 32, 32);
            break;
        case 1:
            bga.map(bank_b.data(), ofs * 32, 32);
            break;
        case 2:
            obja.map(bank_b.data(), (ofs & 0x1) * 32, 32);
            break;
        case 3:
            texture_data.map(bank_b.data(), ofs * 32, 32);
            break;
        default:
            log_fatal("handle mst %d", get_bank_mst(vramcnt[1]));
        }
    }
    
    // remap bank c
    if (get_bank_enabled(vramcnt[2])) {
        u8 ofs = get_bank_offset(vramcnt[2]);
        switch (get_bank_mst(vramcnt[2])) {
        case 0:
            lcdc.map(bank_c.data(), 0x40, 32);
            break;
        case 1:
            bga.map(bank_c.data(), ofs * 32, 32);
            break;
        case 2:
            arm7_vram.map(bank_c.data(), (ofs & 0x1) * 32, 32);
            break;
        case 3:
            texture_data.map(bank_c.data(), ofs * 32, 32);
            break;
        case 4:
            bgb.map(bank_c.data(), 0, 32);
            break;
        default:
            log_fatal("handle mst %d", get_bank_mst(vramcnt[2]));
        }
    }

    if (get_bank_enabled(vramcnt[2]) && (get_bank_mst(vramcnt[2]) == 2)) {
        vramstat |= 1;
    } else {
        vramstat &= ~1;
    }
    
    // remap bank d
    if (get_bank_enabled(vramcnt[3])) {
        u8 ofs = get_bank_offset(vramcnt[3]);
        switch (get_bank_mst(vramcnt[3])) {
        case 0:
            lcdc.map(bank_d.data(), 0x60, 32);
            break;
        case 1:
            bga.map(bank_d.data(), ofs * 32, 32);
            break;
        case 2:
            arm7_vram.map(bank_d.data(), (ofs & 0x1) * 32, 32);
            break;
        case 3:
            texture_data.map(bank_d.data(), ofs * 32, 32);
            break;
        case 4:
            objb.map(bank_d.data(), 0, 32);
            break;
        default:
            log_fatal("handle mst %d", get_bank_mst(vramcnt[3]));
        }
    }

    if (get_bank_enabled(vramcnt[3]) && (get_bank_mst(vramcnt[3]) == 2)) {
        vramstat |= (1 << 1);
    } else {
        vramstat &= ~(1 << 1);
    }
    
    // remap bank e
    if (get_bank_enabled(vramcnt[4])) {
        switch (get_bank_mst(vramcnt[4])) {
        case 0:
            lcdc.map(bank_e.data(), 0x80, 16);
            break;
        case 1:
            bga.map(bank_e.data(), 0, 16);
            break;
        case 2:
            obja.map(bank_e.data(), 0, 16);
            break;
        case 3:
            texture_palette.map(bank_e.data(), 0, 16);
            break;
        case 4:
            // handle extpal later
            break;
        default:
            log_fatal("handle mst %d", get_bank_mst(vramcnt[4]));
        }
    }

    // remap bank f
    if (get_bank_enabled(vramcnt[5])) {
        u8 ofs = get_bank_offset(vramcnt[5]);
        switch (get_bank_mst(vramcnt[5])) {
        case 0:
            lcdc.map(bank_f.data(), 0x90, 4);
            break;
        case 1:
            bga.map(bank_f.data(), (ofs & 0x1) * 0x4 + ((ofs >> 1) & 0x1) * 0x10, 4);
            break;
        case 2:
            obja.map(bank_f.data(), (ofs & 0x1) * 0x4 + ((ofs >> 1) & 0x1) * 0x10, 4);
            break;
        case 3:
            texture_palette.map(bank_f.data(), ((ofs & 0x1) + ((ofs >> 1) & 0x1) * 0x4) * 4, 4);
            break;
        case 4:
            // handle ext palette later
            break;
        case 5:
            // obj ext palette handle later
            break;
        default:
            log_fatal("handle mst %d", get_bank_mst(vramcnt[5]));
        }
    }

    // remap bank g
    if (get_bank_enabled(vramcnt[6])) {
        u8 ofs = get_bank_offset(vramcnt[6]);
        switch (get_bank_mst(vramcnt[6])) {
        case 0:
            lcdc.map(bank_g.data(), 0x94, 4);
            break;
        case 1:
            bga.map(bank_g.data(), (ofs & 0x1) * 0x4 + ((ofs >> 1) & 0x1) * 0x10, 4);
            break;
        case 2:
            obja.map(bank_g.data(), (ofs & 0x1) * 0x4 + ((ofs >> 1) & 0x1) * 0x10, 4);
            break;
        case 3:
            texture_palette.map(bank_g.data(), ((ofs & 0x1) + ((ofs >> 1) & 0x1) * 0x4) * 4, 4);
            break;
        case 4:
            // handle extended palette later
            break;
        case 5:
            // handle extended palette later
            break;
        default:
            log_fatal("handle mst %d", get_bank_mst(vramcnt[6]));
        }
    }

    // remap bank h
    if (get_bank_enabled(vramcnt[7])) {
        switch (get_bank_mst(vramcnt[7])) {
        case 0:
            lcdc.map(bank_h.data(), 0x98, 8);
            break;
        case 1:
            bgb.map(bank_h.data(), 0, 8);
            break;
        case 2:
            // handle extpal later
            break;
        default:
            log_fatal("handle mst %d", get_bank_mst(vramcnt[7]));
        }
    }

    // remap bank i
    if (get_bank_enabled(vramcnt[8])) {
        switch (get_bank_mst(vramcnt[8])) {
        case 0:
            lcdc.map(bank_i.data(), 0xA0, 4);
            break;
        case 1:
            bgb.map(bank_i.data(), 0x8, 4);
            break;
        case 2:
            objb.map(bank_i.data(), 0, 4);
            break;
        case 3:
            // obj extended palette
            // handle later
            break;
        default:
            log_fatal("handle mst %d", get_bank_mst(vramcnt[8]));
        }
    }
}

void VRAM::reset_vram_mapping() {
    lcdc.reset();
    bga.reset();
    obja.reset();
    bgb.reset();
    objb.reset();
    arm7_vram.reset();
    texture_data.reset();
    texture_palette.reset();
}

int VRAM::get_bank_mst(u8 vramcnt) {
    return vramcnt & 0x7;
}

int VRAM::get_bank_offset(u8 vramcnt) {
    return (vramcnt >> 3) & 0x3;
}

bool VRAM::get_bank_enabled(u8 vramcnt) {
    return (vramcnt & (1 << 7));
}