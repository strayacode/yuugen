#include <algorithm>
#include <cctype>
#include "common/StringHelpers.h"

namespace Common {

std::string to_lower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](char c) {
        return std::tolower(c);
    });

    return str;
}

}