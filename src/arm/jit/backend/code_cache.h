#pragma once

#include <array>
#include <memory>
#include "common/types.h"
#include "arm/jit/location.h"

namespace arm {

template <typename T>
class CodeCache {
public:
    void reset() {
        for (int i = 0; i < L1_SIZE; i++) {
            page_table[i] = {};
        }
    }

    bool has_code_at(Location location) {
        auto& l1_entry = page_table[get_l1_index(location)];
        if (!l1_entry) { 
            return false;
        }

        auto& l2_entry = (*l1_entry)[get_l2_index(location)];
        if (!l2_entry) {
            return false;
        }

        return true;
    }

    T& get_or_create(Location location) {
        auto& l1_entry = page_table[get_l1_index(location)];
        if (!l1_entry) { 
            l1_entry = std::make_unique<std::array<L2Entry, L2_SIZE>>();
        }

        auto& l2_entry = (*l1_entry)[get_l2_index(location)];
        if (!l2_entry) {
            l2_entry = std::make_unique<T>();
        }

        return *l2_entry;
    }

    void set(Location location, T value) {
        auto& l1_entry = page_table[get_l1_index(location)];
        if (!l1_entry) {
            l1_entry = std::make_unique<std::array<L2Entry, L2_SIZE>>();
        }

        auto& l2_entry = (*l1_entry)[get_l2_index(location)];
        l2_entry = std::make_unique<T>(value);
    }

private:
    int get_l1_index(Location location) {
        return (location.value >> L1_SHIFT) & L1_MASK;
    }

    int get_l2_index(Location location) {
        return (location.value >> L2_SHIFT) & L2_MASK;
    }

    static constexpr int NUM_BITS = 38;
    static constexpr int L1_BITS = NUM_BITS / 2;
    static constexpr int L1_SHIFT = NUM_BITS - L1_BITS;
    static constexpr int L1_SIZE = 1 << L1_BITS;
    static constexpr u32 L1_MASK = L1_SIZE - 1; 
    static constexpr int L2_BITS = NUM_BITS / 2;
    static constexpr int L2_SHIFT = NUM_BITS - L1_BITS - L2_BITS;
    static constexpr int L2_SIZE = 1 << L2_BITS;
    static constexpr u32 L2_MASK = L2_SIZE - 1;

    using L2Entry = std::unique_ptr<T>;
    using L1Entry = std::unique_ptr<std::array<L2Entry, L2_SIZE>>;
    
    std::array<L1Entry, L1_SIZE> page_table;
};

} // namespace arm