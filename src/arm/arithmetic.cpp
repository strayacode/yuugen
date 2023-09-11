#include "common/bits.h"
#include "common/logger.h"
#include "arm/arithmetic.h"

namespace arm {

std::tuple<u32, std::optional<bool>> lsl(u32 value, int amount) {
    if (amount == 0) {
        return {value, std::nullopt};
    }

    if (amount >= 32) {
        return {0, (amount == 32) && (value & 0x1)};
    }

    return {value << amount, (value >> (32 - amount)) & 0x1};
}

std::tuple<u32, std::optional<bool>> lsr(u32 value, int amount, bool imm) {
    if (amount == 0) {
        if (imm) {
            amount = 32;
        } else {
            return {value, std::nullopt};
        }
    }

    if (amount >= 32) {
        return {0, (amount == 32) && (value >> 31)};
    }

    return {value >> amount, (value >> (amount - 1)) & 0x1};
}

std::tuple<u32, std::optional<bool>> asr(u32 value, int amount, bool imm) {
    if (amount == 0) {
        if (imm) {
            amount = 32;
        } else {
            return {value, std::nullopt};
        }
    }

    if (amount >= 32) {
        return {static_cast<s32>(value) >> 31, value >> 31};
    }

    return {static_cast<s32>(value) >> amount, (value >> (amount - 1)) & 0x1};
}

std::tuple<u32, std::optional<bool>> ror(u32 value, int amount) {
    // an immediate shift amount is already special cased by rrx, so we don't have to handle
    // that case in this function
    if (amount == 0) {
        return {value, std::nullopt};
    }

    amount &= 0x1f;
    auto result = common::rotate_right(value, amount);
    return {result, result >> 31};
}

std::tuple<u32, std::optional<bool>> rrx(u32 value, bool carry) {
    logger.debug("rrx value %08x carry %d", value, carry);
    auto msb = carry << 31;
    carry = value & 1;
    logger.debug("rrx computed carry %d", carry);
    return {(value >> 1) | msb, carry};
}

bool calculate_add_overflow(u32 lhs, u32 rhs, u32 result) {
    return (~(lhs ^ rhs) & (rhs ^ result)) >> 31;
}

bool calculate_sub_overflow(u32 lhs, u32 rhs, u32 result) {
    return ((lhs ^ rhs) & (lhs ^ result)) >> 31;
}

} // namespace arm