#include "common/logger.h"
#include "arm/jit/backend/a64/assembler.h"

namespace arm {

void A64Assembler::reset() {
    buffer.clear();
}

void A64Assembler::dump() {
    logger.debug("buffer:");
    for (const auto& instruction : buffer) {
        logger.debug("%08x", instruction);
    }
}

void A64Assembler::ret() {
    ret(XReg{30});
}

void A64Assembler::ret(XReg rn) {
    emit(0x3597c0 << 10 | rn.id << 5);
}

void A64Assembler::stp(XReg xt1, XReg xt2, XReg xn, IndexMode index_mode, SignedOffset<10, 3> imm) {
    switch (index_mode) {
    case IndexMode::Pre:
        emit(0x2a6 << 22 | imm.offset << 15 | xt2.id << 10 | xn.id << 5 | xt1.id);
        break;
    case IndexMode::Post:
        emit(0x2a2 << 22 | imm.offset << 15 | xt2.id << 10 | xn.id << 5 | xt1.id);
        break;
    }
}

void A64Assembler::stp(WReg wt1, WReg wt2, XReg xn, IndexMode index_mode, SignedOffset<9, 2> imm) {
    switch (index_mode) {
    case IndexMode::Pre:
        emit(0xa6 << 22 | imm.offset << 15 | wt2.id << 10 | xn.id << 5 | wt1.id);
        break;
    case IndexMode::Post:
        emit(0xa2 << 22 | imm.offset << 15 | wt2.id << 10 | xn.id << 5 | wt1.id);
        break;
    }
}

void A64Assembler::stp(XReg xt1, XReg xt2, XReg xn, SignedOffset<10, 3> imm) {
    emit(0x2a4 << 22 | imm.offset << 15 | xt2.id << 10 | xn.id << 5 | xt1.id);
}

void A64Assembler::stp(WReg wt1, WReg wt2, XReg xn, SignedOffset<9, 2> imm) {
    emit(0xa4 << 22 | imm.offset << 15 | wt2.id << 10 | xn.id << 5 | wt1.id);
}

void A64Assembler::emit(u32 data) {
    buffer.push_back(data);
}

} // namespace arm