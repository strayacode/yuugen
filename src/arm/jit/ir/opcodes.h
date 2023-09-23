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
    BitwiseNot,
    BitwiseExclusiveOr,
    LogicalShiftLeft,
    LogicalShiftRight,
    BarrelShifterLogicalShiftLeft,
    BarrelShifterLogicalShiftRight,
    BarrelShifterArithmeticShiftRight,
    BarrelShifterRotateRight,
    BarrelShifterRotateRightExtended,

    // arithmetic opcodes
    Add,
    Subtract,

    // flag opcodes
    Compare,

    // branch opcodes
    Branch,
    BranchExchange,

    // misc opcodes
    Copy,
    
    MemoryWrite,
    MemoryRead,
    Multiply,
    AddCarry,
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

enum class CompareType {
    Equal,
    LessThan,
    IntegerLessThan,
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

static std::string exchange_type_to_string(ExchangeType exchange_type) {
    switch (exchange_type) {
    case ExchangeType::Bit0:
        return "bit0";
    }
}

static std::string compare_type_to_string(CompareType access_type) {
    switch (access_type) {
    case CompareType::Equal:
        return "eq";
    case CompareType::LessThan:
        return "lt";
    case CompareType::IntegerLessThan:
        return "ilt";
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

struct IRBitwiseNot : IROpcode {
    IRBitwiseNot(IRVariable dst, IRValue src) : IROpcode(IROpcodeType::BitwiseNot), dst(dst), src(src) {}

    std::string to_string() override {
        return common::format("%s = not(%s)", dst.to_string().c_str(), src.to_string().c_str());
    }

    IRVariable dst;
    IRValue src;
};

struct IRBitwiseExclusiveOr : IROpcode {
    IRBitwiseExclusiveOr(IRVariable dst, IRValue lhs, IRValue rhs) : IROpcode(IROpcodeType::BitwiseExclusiveOr), dst(dst), lhs(lhs), rhs(rhs) {}

    std::string to_string() override {
        return common::format("%s = xor(%s, %s)", dst.to_string().c_str(), lhs.to_string().c_str(), rhs.to_string().c_str());
    }

    IRVariable dst;
    IRValue lhs;
    IRValue rhs;
};

struct IRLogicalShiftLeft : IROpcode {
    IRLogicalShiftLeft(IRVariable dst, IRValue src, IRValue amount) : IROpcode(IROpcodeType::LogicalShiftLeft), dst(dst), src(src), amount(amount) {}

    std::string to_string() {
        return common::format("%s = lsl(%s, %s)", dst.to_string().c_str(), src.to_string().c_str(), amount.to_string().c_str());
    }

    IRVariable dst;
    IRValue src;
    IRValue amount;
};

struct IRLogicalShiftRight : IROpcode {
    IRLogicalShiftRight(IRVariable dst, IRValue src, IRValue amount) : IROpcode(IROpcodeType::LogicalShiftRight), dst(dst), src(src), amount(amount) {}

    std::string to_string() {
        return common::format("%s = lsr(%s, %s)", dst.to_string().c_str(), src.to_string().c_str(), amount.to_string().c_str());
    }

    IRVariable dst;
    IRValue src;
    IRValue amount;
};

struct IRBarrelShifterLogicalShiftLeft : IROpcode {
    IRBarrelShifterLogicalShiftLeft(IRResultAndCarry result_and_carry, IRValue src, IRValue amount) : IROpcode(IROpcodeType::BarrelShifterLogicalShiftLeft), result_and_carry(result_and_carry), src(src), amount(amount) {}

    std::string to_string() {
        return common::format("%s = barrel_shifter_lsl(%s, %s)", result_and_carry.to_string().c_str(), src.to_string().c_str(), amount.to_string().c_str());
    }

    IRResultAndCarry result_and_carry;
    IRValue src;
    IRValue amount;
};

struct IRBarrelShifterLogicalShiftRight : IROpcode {
    IRBarrelShifterLogicalShiftRight(IRResultAndCarry result_and_carry, IRValue src, IRValue amount) : IROpcode(IROpcodeType::BarrelShifterLogicalShiftRight), result_and_carry(result_and_carry), src(src), amount(amount) {}

    std::string to_string() {
        return common::format("%s = barrel_shifter_lsr(%s, %s)", result_and_carry.to_string().c_str(), src.to_string().c_str(), amount.to_string().c_str());
    }

    IRResultAndCarry result_and_carry;
    IRValue src;
    IRValue amount;
};

struct IRBarrelShifterArithmeticShiftRight : IROpcode {
    IRBarrelShifterArithmeticShiftRight(IRResultAndCarry result_and_carry, IRValue src, IRValue amount) : IROpcode(IROpcodeType::BarrelShifterArithmeticShiftRight), result_and_carry(result_and_carry), src(src), amount(amount) {}

    std::string to_string() {
        return common::format("%s = barrel_shifter_asr(%s, %s)", result_and_carry.to_string().c_str(), src.to_string().c_str(), amount.to_string().c_str());
    }

    IRResultAndCarry result_and_carry;
    IRValue src;
    IRValue amount;
};

struct IRBarrelShifterRotateRight : IROpcode {
    IRBarrelShifterRotateRight(IRResultAndCarry result_and_carry, IRValue src, IRValue amount) : IROpcode(IROpcodeType::BarrelShifterRotateRight), result_and_carry(result_and_carry), src(src), amount(amount) {}

    std::string to_string() {
        return common::format("%s = barrel_shifter_ror(%s, %s)", result_and_carry.to_string().c_str(), src.to_string().c_str(), amount.to_string().c_str());
    }

    IRResultAndCarry result_and_carry;
    IRValue src;
    IRValue amount;
};

struct IRBarrelShifterRotateRightExtended : IROpcode {
    IRBarrelShifterRotateRightExtended(IRResultAndCarry result_and_carry, IRValue src, IRConstant amount, IRValue carry) : IROpcode(IROpcodeType::BarrelShifterRotateRightExtended), result_and_carry(result_and_carry), src(src), amount(amount), carry(carry) {}

    std::string to_string() {
        return common::format("%s = barrel_shifter_rrx(%s, %s, %s)", result_and_carry.to_string().c_str(), src.to_string().c_str(), amount.to_string().c_str(), carry.to_string().c_str());
    }

    IRResultAndCarry result_and_carry;
    IRValue src;
    IRConstant amount;
    IRValue carry;
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

struct IRSubtract : IROpcode {
    IRSubtract(IRVariable dst, IRValue lhs, IRValue rhs) : IROpcode(IROpcodeType::Subtract), dst(dst), lhs(lhs), rhs(rhs) {}

    std::string to_string() override {
        return common::format("%s = subtract(%s, %s)", dst.to_string().c_str(), lhs.to_string().c_str(), rhs.to_string().c_str());
    }

    IRVariable dst;
    IRValue lhs;
    IRValue rhs;
};

struct IRMultiply : IROpcode {
    IRMultiply(IRVariable dst, IRValue lhs, IRValue rhs) : IROpcode(IROpcodeType::Multiply), dst(dst), lhs(lhs), rhs(rhs) {}

    std::string to_string() override {
        return common::format("%s = multiply(%s, %s)", dst.to_string().c_str(), lhs.to_string().c_str(), rhs.to_string().c_str());
    }

    IRVariable dst;
    IRValue lhs;
    IRValue rhs;
};

struct IRCompare : IROpcode {
    IRCompare(IRVariable dst, IRValue lhs, IRValue rhs, CompareType compare_type) : IROpcode(IROpcodeType::Compare), dst(dst), lhs(lhs), rhs(rhs), compare_type(compare_type) {}

    std::string to_string() override {
        return common::format("%s = compare_%s(%s, %s)", dst.to_string().c_str(), compare_type_to_string(compare_type).c_str(), lhs.to_string().c_str(), rhs.to_string().c_str());
    }

    IRVariable dst;
    IRValue lhs;
    IRValue rhs;
    CompareType compare_type;
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
        return common::format("branch_exchange_%s(%s)", exchange_type_to_string(exchange_type).c_str(), address.to_string().c_str());
    }

    IRValue address;
    ExchangeType exchange_type;
};

struct IRCopy : IROpcode {
    IRCopy(IRVariable dst, IRValue src) : IROpcode(IROpcodeType::Copy), dst(dst), src(src) {}

    std::string to_string() override {
        return common::format("%s = copy(%s)", dst.to_string().c_str(), src.to_string().c_str());
    }

    IRVariable dst;
    IRValue src;
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

} // namespace arm