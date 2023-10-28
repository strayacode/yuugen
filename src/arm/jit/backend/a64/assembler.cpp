#include "common/logger.h"
#include "arm/jit/backend/a64/assembler.h"

namespace arm {

void A64Assembler::reset() {
    code.clear();
}

void A64Assembler::dump() {
    logger.debug("code:");
    for (const auto& instruction : code) {
        logger.debug("%08x", instruction);
    }
}

void A64Assembler::ret() {
    ret(XReg{30});
}

void A64Assembler::ret(XReg rn) {
    emit(0x3597c0 << 10 | rn.id << 5);
}

void A64Assembler::emit(u32 data) {
    code.push_back(data);
}

} // namespace arm