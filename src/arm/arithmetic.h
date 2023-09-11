#pragma once

#include <optional>
#include <tuple>
#include "common/types.h"

namespace arm {

std::tuple<u32, std::optional<bool>> lsl(u32 value, int amount);
std::tuple<u32, std::optional<bool>> lsr(u32 value, int amount, bool imm);
std::tuple<u32, std::optional<bool>> asr(u32 value, int amount, bool imm);
std::tuple<u32, std::optional<bool>> ror(u32 value, int amount);
std::tuple<u32, bool> rrx(u32 value, int amount, bool carry);

bool calculate_add_overflow(u32 lhs, u32 rhs, u32 result);
bool calculate_sub_overflow(u32 lhs, u32 rhs, u32 result);

} // namespace arm