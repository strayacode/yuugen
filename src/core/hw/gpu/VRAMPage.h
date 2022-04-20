#pragma once

#include <vector>
#include "common/log.h"
#include "common/types.h"

class VRAMPage {
public:
    VRAMPage() {
        Reset();
    }

    void Reset() {
        banks.clear();
    }

    void AddBank(u8* pointer) {
        banks.push_back(pointer);
    }

    template <typename T>
    T Read(u32 addr) {
        T data = 0;

        for (u64 i = 0; i < banks.size(); i++) {
            T bank_data = 0;
            memcpy(&bank_data, &banks[i][addr & 0xFFF], sizeof(T));

            data |= bank_data;
        }

        return data;
    }

    template <typename T>
    void Write(u32 addr, T data) {
        for (u64 i = 0; i < banks.size(); i++) {
            memcpy(&banks[i][addr & 0xFFF], &data, sizeof(T));
        }
    }

    std::vector<u8*> banks;
};