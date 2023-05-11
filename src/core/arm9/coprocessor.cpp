#include "common/logger.h"
#include "core/arm9/coprocessor.h"
#include "core/arm9/memory.h"

namespace core {

ARM9Coprocessor::ARM9Coprocessor(ARM9Memory& memory) : memory(memory) {}

void ARM9Coprocessor::reset() {
    control.data = 0;
    dtcm.fill(0);
    itcm.fill(0);
    dtcm_control.data = 0;
    itcm_control.data = 0;
}

u32 ARM9Coprocessor::read(u32 cn, u32 cm, u32 cp) {
    switch ((cn << 16) | (cm << 8) | cp) {
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
    case 0x090100:
        dtcm_control.data = value;
        memory.dtcm.config.base = dtcm_control.base << 12;
        memory.dtcm.config.limit = 512 << dtcm_control.size;
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

} // namespace core