#pragma once

#include <filesystem>
#include <vector>
#include <string>
#include "common/types.h"

namespace common {

std::vector<std::string> scan_directory(const std::string& path, std::string extension);
bool matches_extension(const std::string& path, const std::string& extension);
std::string get_formatted_size(u64 size);

} // namespace common