#pragma once

#include <memory>
#include <string>

namespace common {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-security"

template<typename... Args>
std::string format(const char* pattern, Args... args) {
    char buffer[0x10000];
    std::snprintf(buffer, sizeof(buffer), pattern, std::forward<Args>(args)...);
    return std::string(buffer);
}

#pragma clang diagnostic pop

std::string to_lower(std::string str);

} // namespace common