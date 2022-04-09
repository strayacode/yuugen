#pragma once

#include "common/log.h"
#include "common/types.h"

class VRAMPage {
public:
    VRAMPage() {
        Reset();
    }

    void Reset() {
        for (int i = 0; i < 8; i++) {
            banks[i] = nullptr;
        }

        bank_count = 0;
    }

    void AddBank(u8* pointer) {
        banks[bank_count++] = pointer; 
    }

    template <typename T>
    T Read(u32 addr) {
        T data = 0;

        for (int i = 0; i < bank_count; i++) {
            T bank_data = 0;
            memcpy(&bank_data, &banks[i][addr & 0xFFF], sizeof(T));

            data |= bank_data;
        }

        return data;
    }

    template <typename T>
    void Write(u32 addr, T data) {
        for (int i = 0; i < bank_count; i++) {
            memcpy(&banks[i][addr & 0xFFF], &data, sizeof(T));
        }
    }

    std::array<u8*, 8> banks;
    int bank_count;
};