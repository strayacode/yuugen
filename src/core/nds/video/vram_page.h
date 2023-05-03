#pragma once

#include <vector>
#include "common/types.h"
#include "common/memory.h"

namespace core::nds {

class VRAMPage {
public:
    void reset() {
        banks.clear();
    }

    void add_bank(u8* pointer) {
        banks.push_back(pointer);
    }

    template <typename T>
    T read(u32 addr) {
        T data = 0;
        for (u64 i = 0; i < banks.size(); i++) {
            data |= common::read<T>(&banks[i][addr & PAGE_MASK]);
        }

        return data;
    }

    template <typename T>
    void write(u32 addr, T data) {
        for (u64 i = 0; i < banks.size(); i++) {
            common::write<T>(&banks[i][addr & PAGE_MASK]);
        }
    }

private:
    std::vector<u8*> banks;

    constexpr static int PAGE_SIZE = 0x1000;
    constexpr static int PAGE_MASK = PAGE_SIZE - 1;
};

} // namespace core::nds