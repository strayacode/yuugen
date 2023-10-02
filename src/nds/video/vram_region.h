#pragma once

#include <array>
#include "common/types.h"
#include "common/logger.h"
#include "nds/video/vram_page.h"

namespace nds {

class VRAMRegion {
public:
    void reset() {
        for (auto& page : pages) {
            page.reset();
        }
    }

    template <typename T>
    T read(u32 addr) {
        return get_vram_page(addr).template read<T>(addr);
    }

    template <typename T>
    void write(u32 addr, T data) {
        get_vram_page(addr).template write<T>(addr, data);
    }

    void allocate(u32 size) {
        pages.clear();

        auto pages_to_allocate = size / PAGE_SIZE;
        for (u64 i = 0; i < pages_to_allocate; i++) {
            VRAMPage page;
            pages.push_back(page);
        }
    }

    void map(u8* pointer, u32 offset, u32 length) {
        auto pages_to_map = length / PAGE_SIZE;
        for (u64 i = 0; i < pages_to_map; i++) {
            auto index = (offset / PAGE_SIZE) + i;
            pages[index].add_bank(pointer + (i * PAGE_SIZE));
        }
    }

private:
    VRAMPage& get_vram_page(u32 addr) {
        addr &= 0xffffff;

        int region = (addr >> 20) & 0xf;
        u32 offset = addr - (region * 0x100000);
        int index = offset >> 12;
        return pages[index];
    }

    constexpr static int PAGE_SIZE = 0x1000;
    
    std::vector<VRAMPage> pages;
};

} // namespace nds