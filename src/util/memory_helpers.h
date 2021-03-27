#pragma once

#define in_range(lower_bound, size, addr) (addr >= (unsigned)lower_bound) && (addr < (unsigned)(lower_bound + size))