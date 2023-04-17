#pragma once

#include <memory>
#include <string>

namespace common {

template<typename... Args>
std::string format(const std::string& format, Args... args) {
    int size = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;

    std::unique_ptr<char[]> buffer = std::make_unique<char[]>(size);

    std::snprintf(buffer.get(), size, format.c_str(), args...);
    return std::string(buffer.get(), buffer.get() + size - 1);
}

std::string to_lower(std::string str);

} // namespace common