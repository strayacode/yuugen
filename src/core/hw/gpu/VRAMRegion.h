#pragma once

#include <array>
#include "Common/Types.h"
#include "core/hw/gpu/VRAMPage.h"

// size template gets specified as number of kbs
template <int size>
class VRAMRegion {
public:
    void reset() {
        for (int i = 0; i < num_pages; i++) {
            pages[i].Reset();
        }
    }

    template <typename T>
    T read(u32 addr) {
        u8 region = (addr >> 20) & 0xF;
        u32 offset = addr - (region * 0x100000) - 0x06000000;
        int page_index = offset >> 12;

        T data = 0;

        for (u64 i = 0; i < pages[page_index].banks.size(); i++) {
            T bank_data = 0;
            memcpy(&bank_data, &pages[page_index].banks[i][addr & 0xFFF], sizeof(T));

            data |= bank_data;
        }

        return data;
    }

    template <typename T>
    void write(u32 addr, T data) {
        u8 region = (addr >> 20) & 0xF;
        u32 offset = addr - (region * 0x100000) - 0x06000000;
        int page_index = offset >> 12;

        for (u64 i = 0; i < pages[page_index].banks.size(); i++) {
            memcpy(&pages[page_index].banks[i][addr & 0xFFF], &data, sizeof(T));
        }
    }

    void map(u8* pointer, int offset, int pages_to_map) {
        for (int i = 0; i < pages_to_map; i++) {
            pages[offset + i].AddBank(pointer + (i * 0x1000));
        }
    }

private:
    constexpr static int page_size = 0x1000;
    constexpr static int num_pages = size / (page_size / 0x400);

    std::array<VRAMPage, num_pages> pages;
};