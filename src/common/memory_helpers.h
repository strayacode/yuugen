#pragma once

#define in_range(lower_bound, size, addr) (addr >= lower_bound) && (addr < (lower_bound + size))