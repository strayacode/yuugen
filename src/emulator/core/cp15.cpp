#include <emulator/core/cp15.h>
#include <stdio.h>

u32 CP15::read_reg(u32 cn, u32 cm, u32 cp) {
    switch (cn << 16 | cm << 8 | cp) {
    case 0x000000:
        // on arm9 main_id is always 0x41059461
        return 0x41059461;
    case 0x000001:
        return cache_type;
    case 0x000002:
        return tcm_size;
    case 0x010000:
        return control_register;
    default:
        printf("[CP15] undefined registers read 0, C%d, C%d, %d\n", cn, cm, cp);
        return 0;
    }
}