#pragma once

#include <array>
#include "common/types.h"

namespace arm {

// this class will be in the form of a 2 level page table, to save on space
// since a large chunk of the 32-bit address space gets unused, it will be more memory efficient to do 2 levels
// a template parameter specifies the number of bits per page
template <int N>
class PageTable {
public:
    template <typename T>
    u8* get_pointer(u32 addr) {
        auto& l1_entry = page_table[get_l1_index(addr)];
        if (!l1_entry) {
            return nullptr;
        }

        auto& l2_entry = (*l1_entry)[get_l2_index(addr)];
        if (!l2_entry) {
            return nullptr;
        }

        u32 offset = addr & PAGE_MASK;
        return static_cast<u8*>(l2_entry + offset);
    }

    void map(u32 base, u32 end, u8* pointer, u32 mask) {
        for (u32 addr = base; addr < end; addr += PAGE_SIZE) {
            auto& l1_entry = page_table[get_l1_index(addr)];
            if (!l1_entry) {
                l1_entry = std::make_unique<std::array<L2Entry, L2_SIZE>>();
            }

            auto& l2_entry = (*l1_entry)[get_l2_index(addr)];
            u32 offset = addr & mask;
            l2_entry = pointer + offset;
        }
    }

    void unmap(u32 base, u32 end) {
        for (u32 addr = base; addr < end; addr += PAGE_SIZE) {
            auto& l1_entry = page_table[get_l1_index(addr)];
            if (!l1_entry) {
                return;
            }

            auto& l2_entry = (*l1_entry)[get_l2_index(addr)];
            l2_entry = nullptr;
        }
    }

private:
    int get_l1_index(u32 addr) {
        return addr >> L1_SHIFT;
    }

    int get_l2_index(u32 addr) {
        return (addr >> L2_SHIFT) & L2_MASK;
    }

    static constexpr int PAGE_SIZE = 1 << N;
    static constexpr u32 PAGE_MASK = PAGE_SIZE - 1;
    static constexpr int L1_BITS = (32 - N) / 2;
    static constexpr int L1_SHIFT = 32 - L1_BITS;
    static constexpr int L1_SIZE = 1 << L1_BITS;
    static constexpr u32 L1_MASK = L1_SIZE - 1; 
    static constexpr int L2_BITS = (32 - N) / 2;
    static constexpr int L2_SHIFT = 32 - L1_BITS - L2_BITS;
    static constexpr int L2_SIZE = 1 << L2_BITS;
    static constexpr u32 L2_MASK = L2_SIZE - 1;

    using L2Entry = u8*;
    using L1Entry = std::unique_ptr<std::array<L2Entry, L2_SIZE>>;
    
    std::array<L1Entry, L1_SIZE> page_table;
};

} // arm