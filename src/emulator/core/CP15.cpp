#include <emulator/core/CP15.h>
#include <stdio.h>
#include <emulator/common/log.h>
#include <emulator/common/arithmetic.h>

u32 CP15::read_reg(u32 cn, u32 cm, u32 cp) {
    switch (cn << 16 | cm << 8 | cp) {
    case 0x000000:
        // on arm9 main_id is always 0x41059461
        return 0x41059461;
    case 0x000001:
        // on arm9 cache_type is always 0x0F0D2112
        return 0x0F0D2112;
    case 0x000002:
        // on arm9 this register is 0x00140180
        return 0x00140180;
    case 0x010000:
        return control_register;
    default:
        log_fatal("[CP15] undefined registers read C%d, C%d, C%d", cn, cm, cp);
    }
}

void CP15::write_reg(u32 cn, u32 cm, u32 cp, u32 data) {
    switch (cn << 16 | cm << 8 | cp) {
    case 0x010000:
        control_register = data;
        break;
    case 0x090100:
        // write to raw register
        dtcm_reg = data;

        // change dtcm_base_addr and dtcm_size accordingly
        dtcm_size = 512 << (get_bit_range(1, 5, dtcm_reg)); // 512 SHL N
        dtcm_base_addr = get_bit_range(12, 31, dtcm_reg) << 12; // X SHL 12 base must be size aligned
        log_debug("dtcm size: 0x%04x dtcm_base_addr: 0x%04x", dtcm_size, dtcm_base_addr);
        break;
    case 0x090101:
        // write to raw register
        itcm_reg = data;
        // change itcm_size accordingly
        itcm_size = 512 << (get_bit_range(1, 5, itcm_reg)); // 512 SHL N
        log_debug("itcm size: 0x%04x itcm_base_addr: 0x%04x", itcm_size, itcm_base_addr);
        // but itcm_base_addr always remains 0x00000000 so we will not change that
        break;
    default:
        log_fatal("[CP15] undefined register write C%d, C%d, C%d", cn, cm, cp);
    }
}

bool CP15::get_dtcm_enabled() {
    return get_bit(16, control_register);
}

bool CP15::get_itcm_enabled() {
    return get_bit(18, control_register);
}

u32 CP15::get_dtcm_addr() {
    return dtcm_base_addr;
}

u32 CP15::get_dtcm_size() {
    return dtcm_size;
}

u32 CP15::get_itcm_size() {
    return itcm_size;
}

void CP15::direct_boot() {
    // configure control register (enable some stuff including dtcm and itcm and instruction cache) (bits 3..6 are always 1 and bits 0, 2, 7, 12..19 are r/w) the rest are 0
    write_reg(1, 0, 0, 0x0005707D);

    // configure data tcm base
    // details:
    // bits 1..5 specify virtual size as 512 << (N) or 512 SHL N. minimum is 3 (4kb) and maximum is 23 (4gb)
    // bits 12..31 
    write_reg(9, 1, 0, 0x0300000A);

    // configure instruction tcm size
    write_reg(9, 1, 1, 0x00000020);
}