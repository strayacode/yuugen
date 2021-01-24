#pragma once

#define get_bit(index, data) ((data & (1 << index)) != 0)

#define rotate_right(data, shift_amount) ((data >> shift_amount) | (data << (32 - shift_amount)))