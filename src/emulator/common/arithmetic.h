#pragma once
#include <emulator/common/types.h>

u32 get_bit(u32 index, u32 data);

u32 get_bit_range(u32 lower_bound, u32 upper_bound, u32 data);

u32 rotate_right(u32 value, int shift);