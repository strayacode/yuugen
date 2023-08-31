#pragma once

#include <unordered_map>

namespace arm {

template <typename T>
struct CodeCache {
    void reset() {
        code_map.clear();
    }

    bool has_code_at(Location location) {
        return code_map.find(location.value) != code_map.end();
    }
private:
    // maps a location to some compiled code T
    std::unordered_map<u64, T> code_map;
};

} // namespace arm