#include <algorithm>
#include <common/log.h>
#include <string.h>
#include <core/hw/cp15/cp15.h>
#include <core/core.h>

CP15::CP15(System& system) : system(system) {}

void CP15::Reset() {
    memset(itcm, 0, 0x8000);
    memset(dtcm, 0, 0x4000);

    control_register = 0;
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

        // update the itcm and dtcm memory map, as now flags may have changed
        system.arm9_memory.UpdateMemoryMap(0, itcm_size);
        system.arm9_memory.UpdateMemoryMap(dtcm_base, dtcm_base + dtcm_size);
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
        system.cpu_core[1].Halt();
        break;
    case 0x070500:
        // invalidate entire instruction cache
        break;
    case 0x070501:
        // invalidate instruction cache line
        break;
    case 0x070600:
        // invalidate entire data cache
        break;
    case 0x070601: case 0x070602:
        // invalidate data cache line
        break;
    case 0x070A01: case 0x070A02:
        // clean data cache line
        break;
    case 0x070A04:
        // drain write buffer
        break;
    case 0x070E01:
        // clean and invalidate data cache line
        break;
    case 0x070E02:
        // clean and invalidate data cache line
        break;
    case 0x090100: {
        // keep old copy of base and size to unmap certain areas
        u32 old_dtcm_base = dtcm_base;
        u32 old_dtcm_size = dtcm_size;

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

        // now make sure to remap dtcm
        system.arm9_memory.UpdateMemoryMap(old_dtcm_base, old_dtcm_base + old_dtcm_size);
        system.arm9_memory.UpdateMemoryMap(dtcm_base, dtcm_base + dtcm_size);

        log_debug("[CP15]\nDtcm Size: 0x%08x\nDtcm Base: 0x%08x", dtcm_size, dtcm_base);
        break;
    }
    case 0x090101: {
        // keep old copy of size to unmap certain areas
        u32 old_itcm_size = itcm_size;

        // write to raw register
        itcm_reg = data;

        // change itcm_size accordingly
        itcm_size = (itcm_reg >> 1) & 0x1F;

        // itcm size 512 shl n
        itcm_size = 512 << itcm_size;

        // now make sure to remap itcm
        system.arm9_memory.UpdateMemoryMap(0, std::max(old_itcm_size, itcm_size));
        
        log_debug("[CP15]\nItcm Size: 0x%08x\nItcm Base: 0", itcm_size);
        break;
    }
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

bool CP15::GetITCMWriteEnabled() {
    return (control_register & (1 << 18));
}

bool CP15::GetDTCMWriteEnabled() {
    return (control_register & (1 << 16));
}

bool CP15::GetITCMReadEnabled() {
    return !(control_register & (1 << 19)) && (control_register & (1 << 18));
}

bool CP15::GetDTCMReadEnabled() {
    return !(control_register & (1 << 17)) && (control_register & (1 << 16));
}

u32 CP15::GetExceptionBase() {
    return ((control_register & (1 << 13)) ? 0xFFFF0000 : 0x00000000);
}