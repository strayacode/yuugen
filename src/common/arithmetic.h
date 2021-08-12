#pragma once

#define rotate_right(data, shift_amount) ((data) >> (shift_amount)) | ((data) << (32 - (shift_amount)))