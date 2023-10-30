#include "common/logger.h"
#include "arm/jit/backend/a64/assembler.h"

namespace arm {

A64Assembler::A64Assembler(u32* code) : code(code) {}

void A64Assembler::dump() {
    logger.print("buffer:");
    for (int i = 0; i < num_instructions; i++) {
        logger.print("%08x", code[i]);
    }
}

void A64Assembler::ldp(WReg wt1, WReg wt2, XReg xn, IndexMode index_mode, SignedOffset<9, 2> imm) {
    switch (index_mode) {
    case IndexMode::Pre:
        emit(0xa7 << 22 | imm.value << 15 | wt2.id << 10 | xn.id << 5 | wt1.id);
        break;
    case IndexMode::Post:
        emit(0xa3 << 22 | imm.value << 15 | wt2.id << 10 | xn.id << 5 | wt1.id);
        break;
    }
}

void A64Assembler::ldp(XReg xt1, XReg xt2, XReg xn, IndexMode index_mode, SignedOffset<10, 3> imm) {
    switch (index_mode) {
    case IndexMode::Pre:
        emit(0x2a7 << 22 | imm.value << 15 | xt2.id << 10 | xn.id << 5 | xt1.id);
        break;
    case IndexMode::Post:
        emit(0x2a3 << 22 | imm.value << 15 | xt2.id << 10 | xn.id << 5 | xt1.id);
        break;
    }
}

void A64Assembler::ldp(WReg wt1, WReg wt2, XReg xn, SignedOffset<9, 2> imm) {
    emit(0xa5 << 22 | imm.value << 15 | wt2.id << 10 | xn.id << 5 | wt1.id);
}

void A64Assembler::ldp(XReg xt1, XReg xt2, XReg xn, SignedOffset<10, 3> imm) {
    emit(0x2a5 << 22 | imm.value << 15 | xt2.id << 10 | xn.id << 5 | xt1.id);
}

void A64Assembler::mov(WReg wd, u32 imm) {
    if (Immediate16::is_valid(imm)) {
        movz(wd, imm);
        return;
    }

    const u16 lower = imm & 0xffff;
    const u16 upper = (imm >> 16) & 0xffff;

    movz(wd, lower);
    movk(wd, {upper, 16});
}

void A64Assembler::mov(WReg wd, WReg wm) {
    emit(0x150 << 21 | wm.id << 16 | 0x1f << 5 | wd.id);
}

void A64Assembler::mov(XReg xd, XReg xm) {
    emit(0x550 << 21 | xm.id << 16 | 0x1f << 5 | xd.id);
}

void A64Assembler::movz(WReg wd, Immediate16 imm) {
    emit(0xa5 << 23 | imm.value << 5 | wd.id);
}

void A64Assembler::movz(XReg xd, Immediate16 imm) {
    emit(0x1a5 << 23 | imm.value << 5 | xd.id);
}

void A64Assembler::movk(WReg wd, Immediate16 imm) {
    emit(0xe5 << 23 | imm.value << 5 | wd.id);
}

void A64Assembler::movk(XReg xd, Immediate16 imm) {
    emit(0x1e5 << 23 | imm.value << 5 | xd.id);
}

void A64Assembler::ret() {
    ret(XReg{30});
}

void A64Assembler::ret(XReg rn) {
    emit(0x3597c0 << 10 | rn.id << 5);
}

void A64Assembler::stp(WReg wt1, WReg wt2, XReg xn, IndexMode index_mode, SignedOffset<9, 2> imm) {
    switch (index_mode) {
    case IndexMode::Pre:
        emit(0xa6 << 22 | imm.value << 15 | wt2.id << 10 | xn.id << 5 | wt1.id);
        break;
    case IndexMode::Post:
        emit(0xa2 << 22 | imm.value << 15 | wt2.id << 10 | xn.id << 5 | wt1.id);
        break;
    }
}

void A64Assembler::stp(XReg xt1, XReg xt2, XReg xn, IndexMode index_mode, SignedOffset<10, 3> imm) {
    switch (index_mode) {
    case IndexMode::Pre:
        emit(0x2a6 << 22 | imm.value << 15 | xt2.id << 10 | xn.id << 5 | xt1.id);
        break;
    case IndexMode::Post:
        emit(0x2a2 << 22 | imm.value << 15 | xt2.id << 10 | xn.id << 5 | xt1.id);
        break;
    }
}

void A64Assembler::stp(WReg wt1, WReg wt2, XReg xn, SignedOffset<9, 2> imm) {
    emit(0xa4 << 22 | imm.value << 15 | wt2.id << 10 | xn.id << 5 | wt1.id);
}

void A64Assembler::stp(XReg xt1, XReg xt2, XReg xn, SignedOffset<10, 3> imm) {
    emit(0x2a4 << 22 | imm.value << 15 | xt2.id << 10 | xn.id << 5 | xt1.id);
}

void A64Assembler::str(WReg wt, XReg xn, IndexMode index_mode, SignedOffset<9, 0> simm) {
    switch (index_mode) {
    case IndexMode::Pre:
        emit(0x5c0 << 21 | simm.value << 12 | 0x3 << 10 | xn.id << 5 | wt.id);
        break;
    case IndexMode::Post:
        emit(0x5c0 << 21 | simm.value << 12 | 0x1 << 10 | xn.id << 5 | wt.id);
        break;
    }
}

void A64Assembler::str(XReg xt, XReg xn, IndexMode index_mode, SignedOffset<9, 0> simm) {
    switch (index_mode) {
    case IndexMode::Pre:
        emit(0x7c0 << 21 | simm.value << 12 | 0x3 << 10 | xn.id << 5 | xt.id);
        break;
    case IndexMode::Post:
        emit(0x7c0 << 21 | simm.value << 12 | 0x1 << 10 | xn.id << 5 | xt.id);
        break;
    }
}

void A64Assembler::sub(WReg wd, WReg wn, SubImmediate imm) {
    emit(0xa2 << 23 | imm.value << 10 | wn.id << 5 | wd.id);
}

void A64Assembler::sub(XReg xd, XReg xn, SubImmediate imm) {
    emit(0x1a2 << 23 | imm.value << 10 | xn.id << 5 | xd.id);
}

void A64Assembler::emit(u32 data) {
    code[num_instructions++] = data;
}

} // namespace arm