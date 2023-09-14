#pragma once

#include <string>
#include "common/string.h"
#include "common/logger.h"
#include "arm/cpu.h"
#include "arm/state.h"
#include "arm/disassembler/disassembler.h"
#include "arm/jit/ir/types.h"

namespace arm {

enum class IROpcodeType {
    Move,
    LoadGPR,
    StoreGPR,
    Add,
    LogicalShiftLeft,
    And,
    LogicalShiftRight,
    MemoryWrite,
    Sub,
    UpdateFlag,
    StoreFlags,
    Compare,
    LoadCPSR,
    LoadSPSR,
    Or,
    StoreCPSR,
    StoreSPSR,
    ArithmeticShiftRight,
    RotateRight,
    MemoryRead,
    Bic,
    Branch,
};

enum class AccessSize {
    Byte,
    Half,
    Word,
};

enum class AccessType {
    Aligned,
    Unaligned,
    Signed,
};

static std::string access_size_to_string(AccessSize access_size) {
    switch (access_size) {
    case AccessSize::Byte:
        return ".b";
    case AccessSize::Half:
        return ".h";
    case AccessSize::Word:
        return ".w";
    }
}

static std::string access_type_to_string(AccessType access_type) {
    switch (access_type) {
    case AccessType::Aligned:
        return ".a";
    case AccessType::Unaligned:
        return ".u";
    case AccessType::Signed:
        return ".s";
    }
}

static std::string flags_to_string(Flags flags) {
    if (flags == 0) {
        return ".none";
    }

    std::string result = ".";
    if (flags & Flags::N) {
        result += "n";
    }

    if (flags & Flags::Z) {
        result += "z";
    }

    if (flags & Flags::C) {
        result += "c";
    }

    if (flags & Flags::V) {
        result += "v";
    }

    return result;
}

static std::string bool_to_string(bool value) {
    if (value) {
        return "true";
    } else {
        return "false";
    }
}

struct IROpcode {
    IROpcode(IROpcodeType type) : type(type) {}

    virtual ~IROpcode() = default;
    virtual std::string to_string() = 0;

    IROpcodeType get_type() {
        return type;
    }

    template <typename T>
    T* as() {
        return reinterpret_cast<T*>(this);
    }

    IROpcodeType type;
};

struct IRMove : IROpcode {
    IRMove(IRVariable dst, IRValue src, bool set_flags) : IROpcode(IROpcodeType::Move), dst(dst), src(src), set_flags(set_flags) {}

    std::string to_string() override {
        return common::format("mov%s %s, %s", set_flags ? ".s" : "", dst.to_string().c_str(), src.to_string().c_str());
    }

    IRVariable dst;
    IRValue src;
    bool set_flags;
};

struct IRLoadGPR : IROpcode {
    IRLoadGPR(IRVariable dst, GuestRegister src) : IROpcode(IROpcodeType::LoadGPR), dst(dst), src(src) {}

    std::string to_string() override {
        return common::format("ld %s, %s", dst.to_string().c_str(), src.to_string().c_str());
    }

    IRVariable dst;
    GuestRegister src;
};

struct IRStoreGPR : IROpcode {
    IRStoreGPR(GuestRegister dst, IRValue src) : IROpcode(IROpcodeType::StoreGPR), dst(dst), src(src) {}

    std::string to_string() override {
        return common::format("st %s, %s", dst.to_string().c_str(), src.to_string().c_str());
    }

    GuestRegister dst;
    IRValue src;
};

struct IRAdd : IROpcode {
    IRAdd(IRVariable dst, IRValue lhs, IRValue rhs, bool set_flags) : IROpcode(IROpcodeType::Add), dst(dst), lhs(lhs), rhs(rhs), set_flags(set_flags) {}

    std::string to_string() override {
        return common::format("add%s %s, %s, %s", set_flags ? ".s" : "", dst.to_string().c_str(), lhs.to_string().c_str(), rhs.to_string().c_str());
    }

    IRVariable dst;
    IRValue lhs;
    IRValue rhs;
    bool set_flags;
};

struct IRLogicalShiftLeft : IROpcode {
    IRLogicalShiftLeft(IRVariable dst, IRValue src, IRValue amount, bool set_carry) : IROpcode(IROpcodeType::LogicalShiftLeft), dst(dst), src(src), amount(amount), set_carry(set_carry) {}

    std::string to_string() {
        return common::format("lsl%s %s, %s, %s", set_carry ? ".s" : "", dst.to_string().c_str(), src.to_string().c_str(), amount.to_string().c_str());
    }

    IRVariable dst;
    IRValue src;
    IRValue amount;
    bool set_carry;
};

struct IRAnd : IROpcode {
    IRAnd(IRVariable dst, IRValue lhs, IRValue rhs, bool set_flags) : IROpcode(IROpcodeType::And), dst(dst), lhs(lhs), rhs(rhs), set_flags(set_flags) {}

    std::string to_string() override {
        return common::format("and%s %s, %s, %s", set_flags ? ".s" : "", dst.to_string().c_str(), lhs.to_string().c_str(), rhs.to_string().c_str());
    }

    IRVariable dst;
    IRValue lhs;
    IRValue rhs;
    bool set_flags;
};

struct IRLogicalShiftRight : IROpcode {
    IRLogicalShiftRight(IRVariable dst, IRValue src, IRValue amount, bool set_carry) : IROpcode(IROpcodeType::LogicalShiftRight), dst(dst), src(src), amount(amount), set_carry(set_carry) {}

    std::string to_string() {
        return common::format("lsr%s %s, %s, %s", set_carry ? ".s" : "", dst.to_string().c_str(), src.to_string().c_str(), amount.to_string().c_str());
    }

    IRVariable dst;
    IRValue src;
    IRValue amount;
    bool set_carry;
};

struct IRMemoryWrite : IROpcode {
    IRMemoryWrite(IRValue addr, IRValue src, AccessSize access_size, AccessType access_type) : IROpcode(IROpcodeType::MemoryWrite), addr(addr), src(src), access_size(access_size), access_type(access_type) {}

    std::string to_string() {
        return common::format("st%s%s %s, %s", access_size_to_string(access_size).c_str(), access_type_to_string(access_type).c_str(), src.to_string().c_str(), addr.to_string().c_str());
    }

    IRValue addr;
    IRValue src;
    AccessSize access_size;
    AccessType access_type;
};

struct IRSub : IROpcode {
    IRSub(IRVariable dst, IRValue lhs, IRValue rhs, bool set_flags) : IROpcode(IROpcodeType::Sub), dst(dst), lhs(lhs), rhs(rhs), set_flags(set_flags) {}

    std::string to_string() override {
        return common::format("sub%s %s, %s, %s", set_flags ? ".s" : "", dst.to_string().c_str(), lhs.to_string().c_str(), rhs.to_string().c_str());
    }

    IRVariable dst;
    IRValue lhs;
    IRValue rhs;
    bool set_flags;
};

struct IRUpdateFlag : IROpcode {
    IRUpdateFlag(Flags flag, bool value) : IROpcode(IROpcodeType::UpdateFlag), flag(flag), value(value) {}

    std::string to_string() override {
        return common::format("upflg%s, %s", flags_to_string(flag).c_str(), bool_to_string(value).c_str());
    }

    Flags flag;
    bool value;
};

struct IRStoreFlags : IROpcode {
    IRStoreFlags(Flags flags) : IROpcode(IROpcodeType::StoreFlags), flags(flags) {}

    std::string to_string() override {
        return common::format("stflg%s", flags_to_string(flags).c_str());
    }

    Flags flags;
};

struct IRCompare : IROpcode {
    IRCompare(IRValue lhs, IRValue rhs) : IROpcode(IROpcodeType::Compare), lhs(lhs), rhs(rhs) {}

    std::string to_string() override {
        return common::format("cmp.s %s, %s", lhs.to_string().c_str(), rhs.to_string().c_str());
    }

    IRValue lhs;
    IRValue rhs;
};

struct IRLoadCPSR : IROpcode {
    IRLoadCPSR(IRVariable dst) : IROpcode(IROpcodeType::LoadCPSR), dst(dst) {}

    std::string to_string() override {
        return common::format("ldcpsr %s", dst.to_string().c_str());
    }

    IRVariable dst;
};

struct IRLoadSPSR : IROpcode {
    IRLoadSPSR(IRVariable dst) : IROpcode(IROpcodeType::LoadSPSR), dst(dst) {}

    std::string to_string() override {
        return common::format("ldspsr %s", dst.to_string().c_str());
    }

    IRVariable dst;
};

struct IROr : IROpcode {
    IROr(IRVariable dst, IRValue lhs, IRValue rhs, bool set_flags) : IROpcode(IROpcodeType::Or), dst(dst), lhs(lhs), rhs(rhs), set_flags(set_flags) {}

    std::string to_string() override {
        return common::format("or%s %s, %s, %s", set_flags ? ".s" : "", dst.to_string().c_str(), lhs.to_string().c_str(), rhs.to_string().c_str());
    }

    IRVariable dst;
    IRValue lhs;
    IRValue rhs;
    bool set_flags;
};

struct IRStoreCPSR : IROpcode {
    IRStoreCPSR(IRValue src) : IROpcode(IROpcodeType::StoreCPSR), src(src) {}

    std::string to_string() override {
        return common::format("stcpsr %s", src.to_string().c_str());
    }

    IRValue src;
};

struct IRStoreSPSR : IROpcode {
    IRStoreSPSR(IRVariable src) : IROpcode(IROpcodeType::StoreSPSR), src(src) {}

    std::string to_string() override {
        return common::format("stspsr %s", src.to_string().c_str());
    }

    IRVariable src;
};

struct IRArithmeticShiftRight : IROpcode {
    IRArithmeticShiftRight(IRVariable dst, IRValue src, IRValue amount, bool set_carry) : IROpcode(IROpcodeType::ArithmeticShiftRight), dst(dst), src(src), amount(amount), set_carry(set_carry) {}

    std::string to_string() {
        return common::format("asr%s %s, %s, %s", set_carry ? ".s" : "", dst.to_string().c_str(), src.to_string().c_str(), amount.to_string().c_str());
    }

    IRVariable dst;
    IRValue src;
    IRValue amount;
    bool set_carry;
};

struct IRRotateRight : IROpcode {
    IRRotateRight(IRVariable dst, IRValue src, IRValue amount, bool set_carry) : IROpcode(IROpcodeType::RotateRight), dst(dst), src(src), amount(amount), set_carry(set_carry) {}

    std::string to_string() {
        return common::format("ror%s %s, %s, %s", set_carry ? ".s" : "", dst.to_string().c_str(), src.to_string().c_str(), amount.to_string().c_str());
    }

    IRVariable dst;
    IRValue src;
    IRValue amount;
    bool set_carry;
};

struct IRMemoryRead : IROpcode {
    IRMemoryRead(IRVariable dst, IRValue addr, AccessSize access_size, AccessType access_type) : IROpcode(IROpcodeType::MemoryRead), dst(dst), addr(addr), access_size(access_size), access_type(access_type) {}

    std::string to_string() {
        return common::format("ld%s%s %s, %s", access_size_to_string(access_size).c_str(), access_type_to_string(access_type).c_str(), dst.to_string().c_str(), addr.to_string().c_str());
    }

    IRVariable dst;
    IRValue addr;
    AccessSize access_size;
    AccessType access_type;
};

struct IRBic : IROpcode {
    IRBic(IRVariable dst, IRValue lhs, IRValue rhs, bool set_flags) : IROpcode(IROpcodeType::Bic), dst(dst), lhs(lhs), rhs(rhs), set_flags(set_flags) {}

    std::string to_string() override {
        return common::format("bic%s %s, %s, %s", set_flags ? ".s" : "", dst.to_string().c_str(), lhs.to_string().c_str(), rhs.to_string().c_str());
    }

    IRVariable dst;
    IRValue lhs;
    IRValue rhs;
    bool set_flags;
};

struct IRBranch : IROpcode {
    IRBranch(IRValue address, bool is_arm) : IROpcode(IROpcodeType::Branch), address(address), is_arm(is_arm) {}

    std::string to_string() override {
        return common::format("b %s", address.to_string().c_str());
    }

    IRValue address;
    bool is_arm;
};

} // namespace arm