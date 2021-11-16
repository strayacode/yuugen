#pragma once

#include <array>
#include <core/arm/jit_common/block.h>

// for now we assume each block holds enough instructions
// for 128 bytes (32 arm instructions or 64 thumb instructions)
struct BlockPage {
    BlockPage() {
        page.fill(nullptr);
    }

    ~BlockPage() {
        // free all blocks in the page
        for (u64 i = 0; i < page.size(); i++) {
            if (page[i]) {
                delete page[i];
            }
        }
    }

    std::array<Block*, 4096> page;
};

// this class is used to manage blocks and keep them cached
class BlockCache {
public:
    BlockCache();
    ~BlockCache();

    Block* AllocateBlock(u32 address);
private:
    std::array<BlockPage*, 8192> cache;
};