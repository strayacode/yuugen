#include "arm/arithmetic.h"

namespace arm {

bool calculate_add_overflow(u32 lhs, u32 rhs, u32 result) {
    return (~(lhs ^ rhs) & (rhs ^ result)) >> 31;
}

bool calculate_sub_overflow(u32 lhs, u32 rhs, u32 result) {
    return ((lhs ^ rhs) & (lhs ^ result)) >> 31;
}

} // namespace arm