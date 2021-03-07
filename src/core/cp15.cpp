#include <core/core.h>
#include <core/cp15.h>

CP15::CP15(Core* core) : core(core) {

}

void CP15::Reset() {
    memset(itcm, 0, 0x8000);
    memset(dtcm, 0, 0x4000);

    control_register = 0;
    itcm_base = 0;
    itcm_size = 0;
    dtcm_base = 0;
    dtcm_size = 0;

    dtcm_reg = 0;
    itcm_reg = 0;
}

u32 CP15::Read(u32 cn, u32 cm, u32 cp) {
    switch ((cn << 16) | (cm << 8) | cp) {
    case 0x000001:
        return 0x0F0D2112; // this is the value used on the nds
    case 0x020000:
        // pu cachability bits for data/unified protection region
        return 0;
    case 0x020001:
        // pu cachability bits for instruction protection region
        return 0;
    case 0x030000:
        // pu cache write-bufferability bits for data protection regions
        return 0;
    case 0x050000:
        return 0;
    case 0x050001: case 0x050002: case 0x050003:
        return 0;
    case 0x060000: case 0x060100: case 0x060200: case 0x060300: case 0x060400: case 0x060500: case 0x060600: case 0x060700:
        // dont do anything lol this is just protection unit region stuff which we dont need to emulate
        return 0;
    case 0x010000:
        return control_register;
        break;
    case 0x090100:
        return dtcm_reg;
    case 0x090101:
        return itcm_reg;
    default:
        log_fatal("undefined register read C%d, C%d, C%d", cn, cm, cp);
    }
}

void CP15::Write(u32 cn, u32 cm, u32 cp, u32 data) {
    switch ((cn << 16) | (cm << 8) | cp) {
    case 0x010000:
        control_register = data;
        break;
    case 0x020000:
        // pu cachability bits for data/unified protection region
        break;
    case 0x020001:
        // pu cachability bits for instruction protection region
        break;
    case 0x030000:
        // pu cache write-bufferability bits for data protection regions
        break;
    case 0x050002:
        // pu extended access permission data/unified protection region
        break;
    case 0x050003:
        // pu extended access permission instruction protection region
        break;
    case 0x060000: case 0x060100: case 0x060200: case 0x060300: case 0x060400: case 0x060500: case 0x060600: case 0x060700:
        // dont do anything lol this is just protection unit region stuff which we dont need to emulate
        break;
    case 0x070004:
        core->arm9.Halt();
    case 0x070500:
        // invalidate entire instruction cache
        break;
    case 0x070501:
        // invalidate instruction cache line
        break;
    case 0x070600:
        // invalidate entire data cache
        break;
    case 0x070A04:
        // drain write buffer
        break;
    case 0x070E01:
        // clean and invalidate data cache line
        break;
    case 0x090100:
        // write to raw register
        dtcm_reg = data;

        // set the base address
        dtcm_base = dtcm_reg >> 12;
        // dtcm base shl 12
        dtcm_base <<= 12;

        // set the size
        dtcm_size = (dtcm_reg >> 1) & 0x1F;

        // dtcm size 512 shl n
        dtcm_size = 512 << dtcm_size;

        log_debug("dtcm size: 0x%04x dtcm_base: 0x%04x", dtcm_size, dtcm_base);
        break;
    case 0x090101:
        // write to raw register
        itcm_reg = data;

        // change itcm_size accordingly
        itcm_size = (itcm_reg >> 1) & 0x1F;

        // itcm size 512 shl n
        itcm_size = 512 << itcm_size;
        
        log_debug("itcm size: 0x%04x itcm_base: 0x%04x", itcm_size, itcm_base);
        // itcm_base_addr always remains 0x00000000 so we will not change that
        break;
    default:
        log_fatal("undefined register write C%d, C%d, C%d with data 0x%08x", cn, cm, cp, data);
    }
}

void CP15::DirectBoot() {
    Write(1, 0, 0, 0x0005707D);

    // set itcm and dtcm bases and sizes
    Write(9, 1, 0, 0x0300000A);
    Write(9, 1, 1, 0x00000020);
}



u32 CP15::GetITCMSize() {
    return itcm_size;
}


u32 CP15::GetDTCMSize() {
    return dtcm_size;
}

u32 CP15::GetDTCMBase() {
    return dtcm_base;
}

bool CP15::GetITCMEnabled() {
    return (control_register & (1 << 18));
}

bool CP15::GetDTCMEnabled() {
    return (control_register & (1 << 16));
}

u32 CP15::GetExceptionBase() {
    return ((control_register & (1 << 13)) ? 0xFFFF0000 : 0x00000000);
}