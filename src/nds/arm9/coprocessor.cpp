#include "common/logger.h"
#include "nds/arm9/coprocessor.h"
#include "nds/arm9/memory.h"

namespace nds {

ARM9Coprocessor::ARM9Coprocessor(std::unique_ptr<arm::CPU>& cpu, ARM9Memory& memory) : cpu(cpu), memory(memory) {}

void ARM9Coprocessor::reset() {
    control.data = 0;
    dtcm.fill(0);
    itcm.fill(0);
    dtcm_control.data = 0;
    itcm_control.data = 0;
}

u32 ARM9Coprocessor::read(u32 cn, u32 cm, u32 cp) {
    switch ((cn << 16) | (cm << 8) | cp) {
    case 0x000001:
        return 0x0f0d2112;
    case 0x010000:
        return control.data;
    case 0x090100:
        return dtcm_control.data;
    case 0x090101:
        return itcm_control.data;
    case 0x020000:
    case 0x020001:
    case 0x030000:
    case 0x050000:
    case 0x050001:
    case 0x050002:
    case 0x050003:
    case 0x060000:
    case 0x060100:
    case 0x060200:
    case 0x060300:
    case 0x060400:
    case 0x060500:
    case 0x060600:
    case 0x060700:
        return 0;
    default:
        logger.error("ARM9Coprocessor: handle register read c%d, c%d, c%d", cn, cm, cp);
    }

    return 0;
}

void ARM9Coprocessor::write(u32 cn, u32 cm, u32 cp, u32 value) {
    switch ((cn << 16) | (cm << 8) | cp) {
    case 0x010000:
        control.data = value;
        memory.dtcm.config.enable_reads = control.dtcm_enable && !control.dtcm_write_only;
        memory.dtcm.config.enable_writes = control.dtcm_enable;
        memory.itcm.config.enable_reads = control.itcm_enable && !control.itcm_write_only;
        memory.itcm.config.enable_writes = control.itcm_enable;
        break;
    case 0x020000:
    case 0x020001:
    case 0x030000:
    case 0x050002:
    case 0x050003:
    case 0x060000:
    case 0x060100:
    case 0x060200:
    case 0x060300:
    case 0x060400:
    case 0x060500:
    case 0x060600:
    case 0x060700:
    case 0x070500:
    case 0x070501:
    case 0x070600:
    case 0x070601:
    case 0x070602:
    case 0x070a01:
    case 0x070a02:
    case 0x070a04:
    case 0x070e01:
    case 0x070e02:
        break;
    case 0x070004:
        cpu->update_halted(true);
        break;
    case 0x090100:
        dtcm_control.data = value;
        memory.dtcm.config.base = dtcm_control.base << 12;
        memory.dtcm.config.limit = memory.dtcm.config.base + (512 << dtcm_control.size);
        break;
    case 0x090101:
        itcm_control.data = value;
        memory.itcm.config.base = 0;
        memory.itcm.config.limit = 512 << itcm_control.size;
        break;
    default:
        logger.error("ARM9Coprocessor: handle register write c%d, c%d, c%d = %08x", cn, cm, cp, value);
    }
}

u32 ARM9Coprocessor::get_exception_base() {
    return control.exception_vector ? 0xffff0000 : 0x00000000;
}

bool ARM9Coprocessor::has_side_effects(u32 cn, u32 cm, u32 cp) {
    switch ((cn << 16) | (cm << 8) | cp) {
    case 0x070004:
    case 0x070802:
        // wait for irq
        return true;
    case 0x070500:
    case 0x070501:
        // invalidate icache
        return true;
    }

    return false;
}

} // namespace nds