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
        logger.error("handle");
        break;
    default:
        logger.error("ARM9Coprocessor: handle register write c%d, c%d, c%d = %08x", cn, cm, cp, value);
    }
}

u32 ARM9Coprocessor::get_exception_base() {
    return 0;
}

u32 ARM9Coprocessor::get_dtcm_base() {
    return dtcm_control.base << 12;
}

u32 ARM9Coprocessor::get_dtcm_size() {
    return 512 << dtcm_control.size;
}

u32 ARM9Coprocessor::get_itcm_base() {
    return 0; 
}

u32 ARM9Coprocessor::get_itcm_size() {
    return 512 << itcm_control.size;
}

} // namespace core