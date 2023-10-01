#pragma once

#include <unordered_map>
#include <cassert>

namespace arm {

template <typename T>
struct CodeCache {
    void reset() {
        code_map.clear();
    }

    bool has_code_at(Location location) {
        return code_map.find(location.value) != code_map.end();
    }

    T& get(Location location) {
        assert(has_code_at(location));
        return code_map[location.value];
    }

    void set(Location location, T value) {
        assert(!has_code_at(location));
        code_map[location.value] = value;
    }

private:
    // maps a location to some compiled code T
    std::unordered_map<u64, T> code_map;
};

} // namespace arm