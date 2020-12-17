#include <emulator/common/types.h>

u32 get_bit(u32 index, u32 data) {
    return ((data & (1 << index)) >> index);
}

u32 get_bit_range(u32 lower_bound, u32 upper_bound, u32 data) {
    u32 result = (data << (31 - upper_bound)) >> (31 - upper_bound + lower_bound);
    return result;
}

// s32 sign_extend(s16 value)
u32 rotate_right(u32 value, int shift) {
    return ((value << (32 - shift)) | (value >> shift));
}