#pragma once

#include <vector>
#include "common/types.h"
#include "common/bits.h"
#include "common/logger.h"
#include "arm/jit/backend/a64/register.h"

namespace arm {

template <int N, int A>
struct SignedOffset {
    SignedOffset(s64 value) {
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

    static bool is_valid(u64 value) {
        return ((value & 0xffff) == value) 
            || ((value & 0xffff0000) == value)
            || ((value & 0xffff00000000) == value)
            || ((value & 0xffff000000000000) == value);
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
    void ldp(WReg wt1, WReg wt2, XReg xn, IndexMode index_mode, SignedOffset<9, 2> imm);
    void ldp(XReg xt1, XReg xt2, XReg xn, IndexMode index_mode, SignedOffset<10, 3> imm);

    // signed offset
    void ldp(WReg wt1, WReg wt2, XReg xn, SignedOffset<9, 2> imm = 0);
    void ldp(XReg xt1, XReg xt2, XReg xn, SignedOffset<10, 3> imm = 0);

    // convenience function to move a 32-bit imm into a register
    void mov(WReg wd, u32 imm);

    void mov(WReg wd, WReg wm);
    void mov(XReg xd, XReg xm);
    void movz(WReg wd, Immediate16 imm);
    void movz(XReg xd, Immediate16 imm);

    void ret();
    void ret(XReg rn);

    // pre/post index
    void stp(WReg wt1, WReg wt2, XReg xn, IndexMode index_mode, SignedOffset<9, 2> imm);
    void stp(XReg xt1, XReg xt2, XReg xn, IndexMode index_mode, SignedOffset<10, 3> imm);

    // signed offset
    void stp(WReg wt1, WReg wt2, XReg xn, SignedOffset<9, 2> imm = 0);
    void stp(XReg xt1, XReg xt2, XReg xn, SignedOffset<10, 3> imm = 0);

    template <typename T>
    T get_current_code() {
        return reinterpret_cast<T>(code);
    }

    u32* get_code() { return code; }
    int get_num_instructions() const { return num_instructions; }

private:
    void emit(u32 data);

    u32* code{nullptr};
    int num_instructions{0};
};

} // namespace