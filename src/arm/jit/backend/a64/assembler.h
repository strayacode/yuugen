#pragma once

#include <vector>
#include "common/types.h"
#include "common/bits.h"
#include "common/logger.h"
#include "arm/jit/backend/a64/register.h"

namespace arm {

template <int N, int A>
struct Offset {
    Offset(s64 value) {
        this->value = (static_cast<u64>(value) & mask) >> A;
    }

    u32 value;
    static constexpr u64 mask = (static_cast<u64>(1) << N) - 1;
};

struct Immediate16 {
    Immediate16(u64 value) {
        int shift = 0;
        while (value != 0) {
            const u16 masked_value = static_cast<u16>(value & 0xffff);
            if (masked_value == value) {
                this->value = (shift << 16) | masked_value;
                return;
            }

            value >>= 16;
            shift++;
        }

        logger.error("value %016lx can't be encoded in an Immediate16", value);
    }

    Immediate16(u16 value, u32 shift) {
        if (shift % 16 != 0 || shift > 48) {
            logger.error("Immediate16: invalid shift %d", shift);
        }

        this->value = (shift << 12) | value;
    }

    static bool is_valid(u64 value) {
        return ((value & 0xffff) == value) 
            || ((value & 0xffff0000) == value)
            || ((value & 0xffff00000000) == value)
            || ((value & 0xffff000000000000) == value);
    }

    u32 value;
};

struct SubImmediate {
    SubImmediate(u64 value) {
        const u16 masked_value = static_cast<u16>(value & 0xfff);
        if ((value & 0xfff) == value) {
            this->value = (0 << 16) | masked_value;
        } else if ((value & 0xfff000) == value) {
            this->value = (1 << 16) | masked_value;
        } else {
            logger.error("value %016lx can't be encoded in an SubImmediate", value);
        }
    }

    static bool is_valid(u64 value) {
        return ((value & 0xfff) == value) || ((value & 0xfff000) == value);
    }

    u32 value;
};

enum class IndexMode {
    Pre,
    Post,
};

class A64Assembler {
public:
    A64Assembler(u32* code);

    void dump();

    // pre/post index
    void ldp(WReg wt1, WReg wt2, XReg xn, IndexMode index_mode, Offset<9, 2> imm);
    void ldp(XReg xt1, XReg xt2, XReg xn, IndexMode index_mode, Offset<10, 3> imm);

    // signed offset
    void ldp(WReg wt1, WReg wt2, XReg xn, Offset<9, 2> imm = 0);
    void ldp(XReg xt1, XReg xt2, XReg xn, Offset<10, 3> imm = 0);

    // convenience function to move a 32-bit imm into a register
    // TODO: see later if we can do optimisations with imm == 0
    void mov(WReg wd, u32 imm);

    void mov(WReg wd, WReg wm);
    void mov(XReg xd, XReg xm);
    void movz(WReg wd, Immediate16 imm);
    void movz(XReg xd, Immediate16 imm);
    void movk(WReg wd, Immediate16 imm);
    void movk(XReg xd, Immediate16 imm);

    void ret();
    void ret(XReg rn);

    // pre/post index
    void stp(WReg wt1, WReg wt2, XReg xn, IndexMode index_mode, Offset<9, 2> imm);
    void stp(XReg xt1, XReg xt2, XReg xn, IndexMode index_mode, Offset<10, 3> imm);

    // signed offset
    void stp(WReg wt1, WReg wt2, XReg xn, Offset<9, 2> imm = 0);
    void stp(XReg xt1, XReg xt2, XReg xn, Offset<10, 3> imm = 0);

    // pre/post index
    void str(WReg wt, XReg xn, IndexMode index_mode, Offset<9, 0> simm);
    void str(XReg xt, XReg xn, IndexMode index_mode, Offset<9, 0> simm);

    // unsigned offset
    void str(WReg wt, XReg xn, Offset<14, 2> pimm = 0);
    void str(XReg xt, XReg xn, Offset<15, 3> pimm = 0);

    void sub(WReg wd, WReg wn, SubImmediate imm);
    void sub(XReg xd, XReg xn, SubImmediate imm);

    template <typename T>
    T get_current_code() {
        // logger.warn("get current code %p %d %p", code, num_instructions, code + num_instructions);
        logger.warn("current code %p", current_code);
        previous_code = current_code;
        return reinterpret_cast<T>(current_code);
    }

    u32* get_code() { return code; }
    int get_num_instructions() const { return num_instructions; }

private:
    void emit(u32 data);

    u32* code{nullptr};
    u32* current_code{nullptr};
    u32* previous_code{nullptr};
    int num_instructions{0};
};

} // namespace