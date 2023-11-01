#include "common/logger.h"
#include "arm/jit/backend/a64/assembler.h"

namespace arm {

A64Assembler::A64Assembler(u32* code) : code(code), current_code(code), previous_code(code) {}

void A64Assembler::dump() {
    logger.print("buffer:");
    u32* curr = previous_code;
    while (curr != current_code) {
        logger.print("%08x", *curr);
        curr++;
    }
}

void A64Assembler::link(Label& label) {
    if (label.instruction != nullptr) {
        logger.debug("link label to %p", current_code);
        logger.debug("original instruction %p", label.instruction);

        label.target = current_code;

        u32 instruction = *label.instruction;
        const uptr diff = reinterpret_cast<uptr>(label.target) - reinterpret_cast<uptr>(label.instruction);
        
        logger.debug("diff is %p", diff);
        switch (common::get_field<24, 8>(instruction)) {
        case 0x14: {
            // b
            const Offset<28, 2> offset = Offset<28, 2>{static_cast<s64>(diff)};
            logger.debug("b offset value %08x", offset.value);
            instruction |= offset.value;
            *label.instruction = instruction;
            break;
        }
        case 0x54: {
            // b cond
            const Offset<21, 2> offset = Offset<21, 2>{static_cast<s64>(diff)};
            logger.debug("b cond offset value %08x", offset.value);
            instruction |= offset.value << 5;
            *label.instruction = instruction;
            break;
        }
        default:
            logger.todo("handle instruction %08x", instruction);
        }

        logger.debug("%08x", *label.instruction);
    }    
}

void A64Assembler::add(WReg wd, WReg wn, WReg wm, Shift shift, u32 amount) {
    emit(0xb << 24 | static_cast<u32>(shift) << 22 | wm.id << 16 | amount << 10 | wn.id << 5 | wd.id);
}

void A64Assembler::add(XReg xd, XReg xn, XReg xm, Shift shift, u32 amount) {
    emit(0x8b << 24 | static_cast<u32>(shift) << 22 | xm.id << 16 | amount << 10 | xn.id << 5 | xd.id);
}

void A64Assembler::_and(WReg wd, WReg wn, WReg wm, Shift shift, u32 amount) {
    emit(0xa << 24 | static_cast<u32>(shift) << 22 | wm.id << 16 | amount << 10 | wn.id << 5 | wd.id);
}

void A64Assembler::_and(XReg xd, XReg xn, XReg xm, Shift shift, u32 amount) {
    emit(0x8a << 24 | static_cast<u32>(shift) << 22 | xm.id << 16 | amount << 10 | xn.id << 5 | xd.id);
}

void A64Assembler::b(Label& label) {
    logger.debug("label corresponds to instruction at %p", current_code);
    label.instruction = current_code;
    emit(0x5 << 26);
}

void A64Assembler::b(Condition condition, Label& label) {
    logger.debug("label corresponds to instruction at %p", current_code);
    label.instruction = current_code;
    emit(0x54 << 24 | condition);
}

void A64Assembler::bl(void* function_address) {
    const uptr diff = reinterpret_cast<uptr>(function_address) - reinterpret_cast<uptr>(current_code);
    const Offset<28, 2> offset = Offset<28, 2>{static_cast<s64>(diff)};
    bl(offset);
}

void A64Assembler::bl(Offset<28, 2> label) {
    emit(0x25 << 26 | label.value);
}

void A64Assembler::ldp(WReg wt1, WReg wt2, XReg xn, IndexMode index_mode, Offset<9, 2> imm) {
    switch (index_mode) {
    case IndexMode::Pre:
        emit(0xa7 << 22 | imm.value << 15 | wt2.id << 10 | xn.id << 5 | wt1.id);
        break;
    case IndexMode::Post:
        emit(0xa3 << 22 | imm.value << 15 | wt2.id << 10 | xn.id << 5 | wt1.id);
        break;
    }
}

void A64Assembler::ldp(XReg xt1, XReg xt2, XReg xn, IndexMode index_mode, Offset<10, 3> imm) {
    switch (index_mode) {
    case IndexMode::Pre:
        emit(0x2a7 << 22 | imm.value << 15 | xt2.id << 10 | xn.id << 5 | xt1.id);
        break;
    case IndexMode::Post:
        emit(0x2a3 << 22 | imm.value << 15 | xt2.id << 10 | xn.id << 5 | xt1.id);
        break;
    }
}

void A64Assembler::ldp(WReg wt1, WReg wt2, XReg xn, Offset<9, 2> imm) {
    emit(0xa5 << 22 | imm.value << 15 | wt2.id << 10 | xn.id << 5 | wt1.id);
}

void A64Assembler::ldp(XReg xt1, XReg xt2, XReg xn, Offset<10, 3> imm) {
    emit(0x2a5 << 22 | imm.value << 15 | xt2.id << 10 | xn.id << 5 | xt1.id);
}

void A64Assembler::ldr(WReg wt, XReg xn, Offset<14, 2> pimm) {
    emit(0x2e5 << 22 | pimm.value << 10 | xn.id << 5 | wt.id);
}

void A64Assembler::ldr(XReg xt, XReg xn, Offset<15, 3> pimm) {
    emit(0x3e5 << 22 | pimm.value << 10 | xn.id << 5 | xt.id);
}

void A64Assembler::lsl(WReg wd, WReg wn, u32 amount) {
    emit(0x14c << 22 | amount << 16 | wn.id << 5 | wd.id);
}

void A64Assembler::lsl(XReg xd, XReg xn, u32 amount) {
    emit(0x34d << 22 | amount << 16 | xn.id << 5 | xd.id);
}

void A64Assembler::lsl(WReg wd, WReg wn, WReg wm) {
    emit(0xd6 << 21 | wm.id << 16 | 0x8 << 10 | wn.id << 5 | wd.id);
}

void A64Assembler::lsl(XReg xd, XReg xn, XReg xm) {
    emit(0x4d6 << 21 | xm.id << 16 | 0x8 << 10 | xn.id << 5 | xd.id);
}

void A64Assembler::lsr(WReg wd, WReg wn, u32 amount) {
    emit(0x14c << 22 | amount << 16 | 0x1f << 10 | wn.id << 5 | wd.id);
}

void A64Assembler::lsr(XReg xd, XReg xn, u32 amount) {
    emit(0x34d << 22 | amount << 16 | 0x3f << 10 | xn.id << 5 | xd.id);
}

void A64Assembler::lsr(WReg wd, WReg wn, WReg wm) {
    emit(0xd6 << 21 | wm.id << 16 | 0x9 << 10 | wn.id << 5 | wd.id);
}

void A64Assembler::lsr(XReg xd, XReg xn, XReg xm) {
    emit(0x4d6 << 21 | xm.id << 16 | 0x9 << 10 | xn.id << 5 | xd.id);
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

void A64Assembler::msr(SystemReg system_reg, XReg xt) {
    emit(0xd51 << 20 | system_reg << 5 | xt.id);
}

void A64Assembler::mvn(WReg wd, WReg wm, Shift shift, u32 amount) {
    emit(0x2a << 24 | static_cast<u32>(shift) << 22 | 0x1 << 21 | wm.id << 16 | amount << 10 | 0x1f << 5 | wd.id);
}

void A64Assembler::mvn(XReg xd, XReg xm, Shift shift, u32 amount) {
    emit(0xaa << 24 | static_cast<u32>(shift) << 22 | 0x1 << 21 | xm.id << 16 | amount << 10 | 0x1f << 5 | xd.id);
}

void A64Assembler::orr(WReg wd, WReg wn, WReg wm, Shift shift, u32 amount) {
    emit(0x2a << 24 | static_cast<u32>(shift) << 22 | wm.id << 16 | amount << 10 | wn.id << 5 | wd.id);
}

void A64Assembler::orr(XReg xd, XReg xn, XReg xm, Shift shift, u32 amount) {
    emit(0xaa << 24 | static_cast<u32>(shift) << 22 | xm.id << 16 | amount << 10 | xn.id << 5 | xd.id);
}

void A64Assembler::ret() {
    ret(XReg{30});
}

void A64Assembler::ret(XReg rn) {
    emit(0x3597c0 << 10 | rn.id << 5);
}

void A64Assembler::stp(WReg wt1, WReg wt2, XReg xn, IndexMode index_mode, Offset<9, 2> imm) {
    switch (index_mode) {
    case IndexMode::Pre:
        emit(0xa6 << 22 | imm.value << 15 | wt2.id << 10 | xn.id << 5 | wt1.id);
        break;
    case IndexMode::Post:
        emit(0xa2 << 22 | imm.value << 15 | wt2.id << 10 | xn.id << 5 | wt1.id);
        break;
    }
}

void A64Assembler::stp(XReg xt1, XReg xt2, XReg xn, IndexMode index_mode, Offset<10, 3> imm) {
    switch (index_mode) {
    case IndexMode::Pre:
        emit(0x2a6 << 22 | imm.value << 15 | xt2.id << 10 | xn.id << 5 | xt1.id);
        break;
    case IndexMode::Post:
        emit(0x2a2 << 22 | imm.value << 15 | xt2.id << 10 | xn.id << 5 | xt1.id);
        break;
    }
}

void A64Assembler::stp(WReg wt1, WReg wt2, XReg xn, Offset<9, 2> imm) {
    emit(0xa4 << 22 | imm.value << 15 | wt2.id << 10 | xn.id << 5 | wt1.id);
}

void A64Assembler::stp(XReg xt1, XReg xt2, XReg xn, Offset<10, 3> imm) {
    emit(0x2a4 << 22 | imm.value << 15 | xt2.id << 10 | xn.id << 5 | xt1.id);
}

void A64Assembler::str(WReg wt, XReg xn, IndexMode index_mode, Offset<9, 0> simm) {
    switch (index_mode) {
    case IndexMode::Pre:
        emit(0x5c0 << 21 | simm.value << 12 | 0x3 << 10 | xn.id << 5 | wt.id);
        break;
    case IndexMode::Post:
        emit(0x5c0 << 21 | simm.value << 12 | 0x1 << 10 | xn.id << 5 | wt.id);
        break;
    }
}

void A64Assembler::str(XReg xt, XReg xn, IndexMode index_mode, Offset<9, 0> simm) {
    switch (index_mode) {
    case IndexMode::Pre:
        emit(0x7c0 << 21 | simm.value << 12 | 0x3 << 10 | xn.id << 5 | xt.id);
        break;
    case IndexMode::Post:
        emit(0x7c0 << 21 | simm.value << 12 | 0x1 << 10 | xn.id << 5 | xt.id);
        break;
    }
}

void A64Assembler::str(WReg wt, XReg xn, Offset<14, 2> pimm) {
    emit(0x2e4 << 22 | pimm.value << 10 | xn.id << 5 | wt.id);
}

void A64Assembler::str(XReg xt, XReg xn, Offset<15, 3> pimm) {
    emit(0x3e4 << 22 | pimm.value << 10 | xn.id << 5 | xt.id);
}

void A64Assembler::sub(WReg wd, WReg wn, SubImmediate imm) {
    emit(0xa2 << 23 | imm.value << 10 | wn.id << 5 | wd.id);
}

void A64Assembler::sub(XReg xd, XReg xn, SubImmediate imm) {
    emit(0x1a2 << 23 | imm.value << 10 | xn.id << 5 | xd.id);
}

void A64Assembler::sub(WReg wd, WReg wn, WReg wm, Shift shift, u32 amount) {
    emit(0x4b << 24 | static_cast<u32>(shift) << 22 | wm.id << 16 | amount << 10 | wn.id << 5 | wd.id);
}

void A64Assembler::sub(XReg xd, XReg xn, XReg xm, Shift shift, u32 amount) {
    emit(0xcb << 24 | static_cast<u32>(shift) << 22 | xm.id << 16 | amount << 10 | xn.id << 5 | xd.id);
}

void A64Assembler::emit(u32 data) {
    logger.debug("emit %08x to %p", data, current_code);
    *current_code++ = data;
    num_instructions++;
}

} // namespace arm