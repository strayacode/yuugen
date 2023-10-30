#pragma once

#include <cstdint>
#include <type_traits>

using u8 = std::uint8_t;
using s8 = std::uint8_t;
using u16 = std::uint16_t;
using s16 = std::int16_t;
using u32 = std::uint32_t;
using s32 = std::int32_t;
using u64 = std::uint64_t;
using s64 = std::int64_t;
using f32 = float;
using f64 = double;
using uptr = std::uintptr_t;

template <typename T, typename... Args> 
struct is_one_of {
  static constexpr bool value = (... || std::is_same_v<T, Args>);
};

template <typename T, typename... Args>
inline constexpr bool is_one_of_v = is_one_of<T, Args...>::value;