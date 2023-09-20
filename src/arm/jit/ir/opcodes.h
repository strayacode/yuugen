#pragma once

#include <string>
#include "common/string.h"
#include "common/logger.h"
#include "arm/cpu.h"
#include "arm/state.h"
#include "arm/disassembler/disassembler.h"
#include "arm/jit/ir/value.h"

namespace arm {

enum class IROpcodeType {
    // state opcodes
    LoadGPR,
    StoreGPR,
    LoadCPSR,
    LoadSPSR,
    StoreCPSR,
    StoreSPSR,

    // bitwise opcodes
    BitwiseAnd,
    BitwiseOr,

    // arithmetic opcodes
    Add,
    Sub,

    // flag opcodes
    GetNZ,
    GetNZCV,

    // misc opcodes
    Copy,
    
    LogicalShiftLeft,
    LogicalShiftRight,
    MemoryWrite,
    UpdateFlag,
    StoreFlags,
    Compare,
    ArithmeticShiftRight,
    RotateRight,
    MemoryRead,
    Bic,
    Branch,
    BranchExchange,
    Multiply,
    ExclusiveOr,
    Test,
    AddCarry,
    MoveNegate,
    CompareNegate,
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

enum class ExchangeType {
    Bit0,
};

static std::string access_size_to_string(AccessSize access_size) {
    switch (access_size) {
    case AccessSize::Byte:
        return "byte";
    case AccessSize::Half:
        return "half";
    case AccessSize::Word:
        return "word";
    }
}

static std::string access_type_to_string(AccessType access_type) {
    switch (access_type) {
    case AccessType::Aligned:
        return "aligned";
    case AccessType::Unaligned:
        return "unaligned";
    case AccessType::Signed:
        return "signed";
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

static std::string exchange_type_to_string(ExchangeType exchange_type) {
    switch (exchange_type) {
    case ExchangeType::Bit0:
        return ".bit0";
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

struct IRLoadGPR : IROpcode {
    IRLoadGPR(IRVariable dst, GuestRegister src) : IROpcode(IROpcodeType::LoadGPR), dst(dst), src(src) {}

    std::string to_string() override {
        return common::format("%s = load_gpr(%s)", dst.to_string().c_str(), src.to_string().c_str());
    }

    IRVariable dst;
    GuestRegister src;
};

struct IRStoreGPR : IROpcode {
    IRStoreGPR(GuestRegister dst, IRValue src) : IROpcode(IROpcodeType::StoreGPR), dst(dst), src(src) {}

    std::string to_string() override {
        return common::format("store_gpr(%s, %s)", dst.to_string().c_str(), src.to_string().c_str());
    }

    GuestRegister dst;
    IRValue src;
};

struct IRLoadCPSR : IROpcode {
    IRLoadCPSR(IRVariable dst) : IROpcode(IROpcodeType::LoadCPSR), dst(dst) {}

    std::string to_string() override {
        return common::format("%s = load_cpsr()", dst.to_string().c_str());
    }

    IRVariable dst;
};

struct IRStoreCPSR : IROpcode {
    IRStoreCPSR(IRValue src) : IROpcode(IROpcodeType::StoreCPSR), src(src) {}

    std::string to_string() override {
        return common::format("store_cpsr(%s)", src.to_string().c_str());
    }

    IRValue src;
};

struct IRLoadSPSR : IROpcode {
    IRLoadSPSR(IRVariable dst) : IROpcode(IROpcodeType::LoadSPSR), dst(dst) {}

    std::string to_string() override {
        return common::format("%s = load_spsr", dst.to_string().c_str());
    }

    IRVariable dst;
};

struct IRStoreSPSR : IROpcode {
    IRStoreSPSR(IRVariable src) : IROpcode(IROpcodeType::StoreSPSR), src(src) {}

    std::string to_string() override {
        return common::format("store_spsr(%s)", src.to_string().c_str());
    }

    IRVariable src;
};

struct IRBitwiseAnd : IROpcode {
    IRBitwiseAnd(IRVariable dst, IRValue lhs, IRValue rhs) : IROpcode(IROpcodeType::BitwiseAnd), dst(dst), lhs(lhs), rhs(rhs) {}

    std::string to_string() override {
        return common::format("%s = and(%s, %s)", dst.to_string().c_str(), lhs.to_string().c_str(), rhs.to_string().c_str());
    }

    IRVariable dst;
    IRValue lhs;
    IRValue rhs;
};

struct IRBitwiseOr : IROpcode {
    IRBitwiseOr(IRVariable dst, IRValue lhs, IRValue rhs) : IROpcode(IROpcodeType::BitwiseOr), dst(dst), lhs(lhs), rhs(rhs) {}

    std::string to_string() override {
        return common::format("%s = or(%s, %s)", dst.to_string().c_str(), lhs.to_string().c_str(), rhs.to_string().c_str());
    }

    IRVariable dst;
    IRValue lhs;
    IRValue rhs;
};

struct IRAdd : IROpcode {
    IRAdd(IRVariable dst, IRValue lhs, IRValue rhs) : IROpcode(IROpcodeType::Add), dst(dst), lhs(lhs), rhs(rhs) {}

    std::string to_string() override {
        return common::format("%s = add(%s, %s)", dst.to_string().c_str(), lhs.to_string().c_str(), rhs.to_string().c_str());
    }

    IRVariable dst;
    IRValue lhs;
    IRValue rhs;
};

struct IRSub : IROpcode {
    IRSub(IRVariable dst, IRValue lhs, IRValue rhs) : IROpcode(IROpcodeType::Sub), dst(dst), lhs(lhs), rhs(rhs) {}

    std::string to_string() override {
        return common::format("%s = sub(%s, %s)", dst.to_string().c_str(), lhs.to_string().c_str(), rhs.to_string().c_str());
    }

    IRVariable dst;
    IRValue lhs;
    IRValue rhs;
};

struct IRGetNZ : IROpcode {
    IRGetNZ(IRVariable dst, IRValue cpsr, IRValue value) : IROpcode(IROpcodeType::GetNZ), dst(dst), cpsr(cpsr), value(value) {}

    std::string to_string() override {
        return common::format("%s = get_nz(%s %s)", dst.to_string().c_str(), cpsr.to_string().c_str(), value.to_string().c_str());
    }

    IRVariable dst;
    IRValue cpsr;
    IRValue value;
};

struct IRGetNZCV : IROpcode {
    IRGetNZCV(IRVariable dst, IRValue cpsr, IRValue value) : IROpcode(IROpcodeType::GetNZCV), dst(dst), cpsr(cpsr), value(value) {}

    std::string to_string() override {
        return common::format("%s = get_nzcv(%s %s)", dst.to_string().c_str(), cpsr.to_string().c_str(), value.to_string().c_str());
    }

    IRVariable dst;
    IRValue cpsr;
    IRValue value;
};

struct IRCopy : IROpcode {
    IRCopy(IRVariable dst, IRValue src) : IROpcode(IROpcodeType::Copy), dst(dst), src(src) {}

    std::string to_string() override {
        return common::format("%s = copy(%s)", dst.to_string().c_str(), src.to_string().c_str());
    }

    IRVariable dst;
    IRValue src;
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
        return common::format("write_%s_%s(%s, %s)", access_size_to_string(access_size).c_str(), access_type_to_string(access_type).c_str(), src.to_string().c_str(), addr.to_string().c_str());
    }

    IRValue addr;
    IRValue src;
    AccessSize access_size;
    AccessType access_type;
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
        return common::format("%s = read_%s_%s(%s)", dst.to_string().c_str(), access_size_to_string(access_size).c_str(), access_type_to_string(access_type).c_str(), addr.to_string().c_str());
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
        return common::format("branch(%s)", address.to_string().c_str());
    }

    IRValue address;
    bool is_arm;
};

struct IRBranchExchange : IROpcode {
    IRBranchExchange(IRValue address, ExchangeType exchange_type) : IROpcode(IROpcodeType::BranchExchange), address(address), exchange_type(exchange_type) {}

    std::string to_string() override {
        return common::format("bx%s %s", exchange_type_to_string(exchange_type).c_str(), address.to_string().c_str());
    }

    IRValue address;
    ExchangeType exchange_type;
};

struct IRMultiply : IROpcode {
    IRMultiply(IRVariable dst, IRValue lhs, IRValue rhs, bool set_flags) : IROpcode(IROpcodeType::Multiply), dst(dst), lhs(lhs), rhs(rhs), set_flags(set_flags) {}

    std::string to_string() override {
        return common::format("mul%s %s, %s, %s", set_flags ? ".s" : "", dst.to_string().c_str(), lhs.to_string().c_str(), rhs.to_string().c_str());
    }

    IRVariable dst;
    IRValue lhs;
    IRValue rhs;
    bool set_flags;
};

struct IRExclusiveOr : IROpcode {
    IRExclusiveOr(IRVariable dst, IRValue lhs, IRValue rhs, bool set_flags) : IROpcode(IROpcodeType::ExclusiveOr), dst(dst), lhs(lhs), rhs(rhs), set_flags(set_flags) {}

    std::string to_string() override {
        return common::format("xor%s %s, %s, %s", set_flags ? ".s" : "", dst.to_string().c_str(), lhs.to_string().c_str(), rhs.to_string().c_str());
    }

    IRVariable dst;
    IRValue lhs;
    IRValue rhs;
    bool set_flags;
};

struct IRTest : IROpcode {
    IRTest(IRValue lhs, IRValue rhs) : IROpcode(IROpcodeType::Test), lhs(lhs), rhs(rhs) {}

    std::string to_string() override {
        return common::format("tst.s %s, %s", lhs.to_string().c_str(), rhs.to_string().c_str());
    }

    IRValue lhs;
    IRValue rhs;
};

struct IRAddCarry : IROpcode {
    IRAddCarry(IRVariable dst, IRValue lhs, IRValue rhs, bool set_flags) : IROpcode(IROpcodeType::AddCarry), dst(dst), lhs(lhs), rhs(rhs), set_flags(set_flags) {}

    std::string to_string() override {
        return common::format("adc%s %s, %s, %s", set_flags ? ".s" : "", dst.to_string().c_str(), lhs.to_string().c_str(), rhs.to_string().c_str());
    }

    IRVariable dst;
    IRValue lhs;
    IRValue rhs;
    bool set_flags;
};

struct IRMoveNegate : IROpcode {
    IRMoveNegate(IRVariable dst, IRValue src, bool set_flags) : IROpcode(IROpcodeType::MoveNegate), dst(dst), src(src), set_flags(set_flags) {}

    std::string to_string() override {
        return common::format("mvn%s %s, %s", set_flags ? ".s" : "", dst.to_string().c_str(), src.to_string().c_str());
    }

    IRVariable dst;
    IRValue src;
    bool set_flags;
};

struct IRCompareNegate : IROpcode {
    IRCompareNegate(IRValue lhs, IRValue rhs) : IROpcode(IROpcodeType::CompareNegate), lhs(lhs), rhs(rhs) {}

    std::string to_string() override {
        return common::format("cmn.s %s, %s", lhs.to_string().c_str(), rhs.to_string().c_str());
    }

    IRValue lhs;
    IRValue rhs;
};

} // namespace arm