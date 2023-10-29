#pragma once

#include <vector>
#include "common/types.h"
#include "arm/jit/backend/a64/register.h"

namespace arm {

template <int N, int A>
struct SignedOffset {
    SignedOffset(s64 value) {
        offset = (static_cast<u64>(value) & mask) >> A;
    }

    u32 offset;
    static constexpr u64 mask = (static_cast<u64>(1) << N) - 1;
};

enum class IndexMode {
    Pre,
    Post,
};

class A64Assembler {
public:
    void reset();
    void dump();

    void ret();
    void ret(XReg rn);

    // pre/post index
    void stp(XReg xt1, XReg xt2, XReg xn, IndexMode index_mode, SignedOffset<10, 3> imm);
    void stp(WReg wt1, WReg wt2, XReg xn, IndexMode index_mode, SignedOffset<9, 2> imm);

    // signed offset
    void stp(XReg xt1, XReg xt2, XReg xn, SignedOffset<10, 3> imm = 0);
    void stp(WReg wt1, WReg wt2, XReg xn, SignedOffset<9, 2> imm = 0);

    std::vector<u32>& get_buffer() { return buffer; }
    void* get_code() { return buffer.data(); }

private:
    void emit(u32 data);

    std::vector<u32> buffer;
};

} // namespace