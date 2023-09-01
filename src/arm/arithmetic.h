#pragma once

#include "common/types.h"

namespace arm {

bool calculate_add_overflow(u32 lhs, u32 rhs, u32 result);
bool calculate_sub_overflow(u32 lhs, u32 rhs, u32 result);

} // namespace arm