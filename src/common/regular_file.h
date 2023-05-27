#pragma once

#include <string>
#include <vector>
#include "common/types.h"

namespace common {

class RegularFile {
public:
    void load(const std::string& path);
    u8* get_pointer(u32 offset);

private:
    std::vector<u8> data;
    u64 size;
};

} // namespace common