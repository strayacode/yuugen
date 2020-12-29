#include <emulator/core/cp15.h>
#include <stdio.h>
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
        printf("[CP15] undefined registers read 0, C%d, C%d, %d\n", cn, cm, cp);
        return 0;
    }
}

bool CP15::get_dtcm_enabled() {
    return get_bit(16, control_register);
}

bool CP15::get_itcm_enabled() {
    return get_bit(18, control_register);
}