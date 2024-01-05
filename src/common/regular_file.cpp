#include <fstream>
#include "common/regular_file.h"
#include "common/logger.h"

namespace common {

void RegularFile::load(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        LOG_ERROR("%s not found", path.c_str());
    }

    std::streampos pos;
    file.seekg(0, std::ios::end);
    size = file.tellg();
    file.seekg(0, std::ios::beg);
    data.reserve(size);
    file.read(reinterpret_cast<char*>(data.data()), size);
    LOG_DEBUG("%s loaded successfully with size %d bytes", path.c_str(), size);
}

u8* RegularFile::get_pointer(u32 offset) {
    return &data[offset];
}

} // namespace common