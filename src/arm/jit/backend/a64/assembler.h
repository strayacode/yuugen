#pragma once

#include <vector>
#include <bit>
#include "common/types.h"
#include "common/bits.h"
#include "common/logger.h"
#include "arm/jit/backend/a64/register.h"
#include "arm/cpu.h"

namespace arm {

template <int N>
struct Immediate {
    Immediate(u64 value) {
        if (!is_valid(value)) {
            logger.error("Assembler: immediate %08x doesn't fit into %d bits", value, N);
        }

        this->value = value;
    }

    bool is_valid(u64 value) {
        return (value & mask) == value;
    }

    u32 value;
    static constexpr u64 mask = (static_cast<u64>(1) << N) - 1;
};

template <int N>
struct BitwiseImmediate {
    BitwiseImmediate(u64 value) {
        // algorithm taken from dolphin
        // replicate 32-bit imm in upper and lower 32-bit
        if (N == 32) {
            value <<= 32;
            value |= value >> 32;
        }

        if (value == 0 || (~value) == 0) {
            logger.todo("BitwiseImmediate: invalid immediate %016lx", value);
        }

        const int rotation = std::countr_zero(value & (value + 1));
        const u64 normalised = std::rotr(value, rotation);

        const int element_size = std::countr_zero(normalised & (normalised + 1));
        const int ones = std::countr_one(normalised);

        if (std::rotr(value, element_size) != value) {
            logger.todo("BitwiseImmediate: invalid immediate %016lx", value);
        }

        const int s = ((-element_size) << 1) | (ones - 1);
        const int r = (element_size - rotation) & (element_size - 1);
        const int n = (~s >> 6) & 0x1;

        this->value = static_cast<u32>((s & 0x3f) | (r << 6) | (n << 12));
    }

    static bool is_valid(u64 value) {
        if (N == 32) {
            value <<= 32;
            value |= value >> 32;
        }

        if (value == 0 || (~value) == 0) {
            return false;
        }

        const int rotation = std::countr_zero(value & (value + 1));
        const u64 normalised = std::rotr(value, rotation);

        const int element_size = std::countr_zero(normalised & (normalised + 1));
        return std::rotr(value, element_size) == value;
    }

    u32 value;
};

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
        if (value == 0) {
            this->value = 0;
            return;
        }

        int shift = 0;
        while (value != 0) {
            const u16 masked_value = static_cast<u16>(value);
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

struct AddSubImmediate {
    AddSubImmediate(u64 value) {
        if ((value & 0xfff) == value) {
            this->value = value;
        } else if ((value & 0xfff000) == value) {
            this->value = (1 << 12) | (value >> 12);
        } else {
            logger.error("value %016lx can't be encoded in an AddSubImmediate", value);
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

struct Label {
    u32* instruction{nullptr};
    u32* target{nullptr};
};

enum class Shift : u32 {
    LSL = 0,
    LSR = 1,
    ASR = 2,
    ROR = 3,
};

class A64Assembler {
public:
    A64Assembler(u32* code);

    void dump();
    void link(Label& label);
    void invoke_function(void* address);

    void add(WReg wd, WReg wn, AddSubImmediate imm);
    void add(XReg xd, XReg xn, AddSubImmediate imm);
    void add(WReg wd, WReg wn, WReg wm, Shift shift = Shift::LSL, u32 amount = 0);
    void add(XReg xd, XReg xn, XReg xm, Shift shift = Shift::LSL, u32 amount = 0);

    void _and(WReg wd, WReg wn, BitwiseImmediate<32> imm);
    void _and(XReg xd, XReg xn, BitwiseImmediate<64> imm);
    void _and(WReg wd, WReg wn, WReg wm, Shift shift = Shift::LSL, u32 amount = 0);
    void _and(XReg xd, XReg xn, XReg xm, Shift shift = Shift::LSL, u32 amount = 0);

    void b(Label& label);
    void b(Condition condition, Label& label);

    void bl(Offset<28, 2> label);
    void blr(XReg xn);

    void cmp(WReg wn, WReg wm, Shift shift = Shift::LSL, u32 amount = 0);
    void cmp(XReg xn, XReg xm, Shift shift = Shift::LSL, u32 amount = 0);
    void cmp(WReg wn, AddSubImmediate imm);
    void cmp(XReg xn, AddSubImmediate imm);

    void cset(WReg wd, Condition condition);
    void cset(XReg xd, Condition condition);

    void eor(WReg wd, WReg wn, BitwiseImmediate<32> imm);
    void eor(XReg xd, XReg xn, BitwiseImmediate<64> imm);
    void eor(WReg wd, WReg wn, WReg wm, Shift shift = Shift::LSL, u32 amount = 0);
    void eor(XReg xd, XReg xn, XReg xm, Shift shift = Shift::LSL, u32 amount = 0);

    // pre/post index
    void ldp(WReg wt1, WReg wt2, XReg xn, IndexMode index_mode, Offset<9, 2> imm);
    void ldp(XReg xt1, XReg xt2, XReg xn, IndexMode index_mode, Offset<10, 3> imm);

    // signed offset
    void ldp(WReg wt1, WReg wt2, XReg xn, Offset<9, 2> imm = 0);
    void ldp(XReg xt1, XReg xt2, XReg xn, Offset<10, 3> imm = 0);

    // unsigned offset
    void ldr(WReg wt, XReg xn, Offset<14, 2> pimm = 0);
    void ldr(XReg xt, XReg xn, Offset<15, 3> pimm = 0);

    void lsl(WReg wd, WReg wn, u32 amount);
    void lsl(XReg xd, XReg xn, u32 amount);
    void lsl(WReg wd, WReg wn, WReg wm);
    void lsl(XReg xd, XReg xn, XReg xm);

    void lsr(WReg wd, WReg wn, u32 amount);
    void lsr(XReg xd, XReg xn, u32 amount);
    void lsr(WReg wd, WReg wn, WReg wm);
    void lsr(XReg xd, XReg xn, XReg xm);

    // convenience function to move a 32-bit imm into a register
    // TODO: see later if we can do optimisations with imm == 0
    void mov(WReg wd, u32 imm);

    // convenience function to move a 64-bit imm into a register
    void mov(XReg xd, u64 imm);

    void mov(WReg wd, WReg wm);
    void mov(XReg xd, XReg xm);
    void movz(WReg wd, Immediate16 imm);
    void movz(XReg xd, Immediate16 imm);
    void movk(WReg wd, Immediate16 imm);
    void movk(XReg xd, Immediate16 imm);

    void msr(SystemReg system_reg, XReg xt);

    void mul(WReg wd, WReg wn, WReg wm);
    void mul(XReg xd, XReg xn, XReg xm);

    void mvn(WReg wd, WReg wm, Shift shift = Shift::LSL, u32 amount = 0);
    void mvn(XReg xd, XReg xm, Shift shift = Shift::LSL, u32 amount = 0);

    void orr(WReg wd, WReg wn, WReg wm, Shift shift = Shift::LSL, u32 amount = 0);
    void orr(XReg xd, XReg xn, XReg xm, Shift shift = Shift::LSL, u32 amount = 0);

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

    void sub(WReg wd, WReg wn, AddSubImmediate imm);
    void sub(XReg xd, XReg xn, AddSubImmediate imm);
    void sub(WReg wd, WReg wn, WReg wm, Shift shift = Shift::LSL, u32 amount = 0);
    void sub(XReg xd, XReg xn, XReg xm, Shift shift = Shift::LSL, u32 amount = 0);

    template <typename T>
    T get_current_code() {
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