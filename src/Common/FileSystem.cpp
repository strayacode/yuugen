#include <cmath>
#include "Common/FileSystem.h"
#include "Common/format.h"

namespace FileSystem {

std::vector<std::string> scan_directory(const std::string& path, std::string extension) {
    std::vector<std::string> entries;

    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (matches_extension(entry.path(), extension)) {
            entries.push_back(entry.path());
        }
    }

    return entries;
}

bool matches_extension(const std::string& path, const std::string& extension) {
    return std::filesystem::path(path).extension() == extension;
}

std::string get_formatted_size(u64 size) {
    int i = 0;
    double mantissa = size;
    static const char* size_types = "BKMGTPE";

    while (mantissa >= 1024) {
        mantissa /= 1024;
        i++;
    }

    mantissa = std::ceil(mantissa * 10.0f) / 10.0f;

    if (i) {
        return format("%.2f %cB", mantissa, size_types[i]);
    }

    return format("%.2f B", mantissa);
}

}