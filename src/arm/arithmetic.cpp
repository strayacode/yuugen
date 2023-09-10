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

bool calculate_add_overflow(u32 lhs, u32 rhs, u32 result) {
    return (~(lhs ^ rhs) & (rhs ^ result)) >> 31;
}

bool calculate_sub_overflow(u32 lhs, u32 rhs, u32 result) {
    return ((lhs ^ rhs) & (lhs ^ result)) >> 31;
}

} // namespace arm