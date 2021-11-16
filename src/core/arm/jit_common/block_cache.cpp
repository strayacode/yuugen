#include <core/arm/jit_common/block_cache.h>

BlockCache::BlockCache() {
    cache.fill(nullptr);
}

BlockCache::~BlockCache() {
    // free all block pages
    for (u64 i = 0; i < cache.size(); i++) {
        if (cache[i]) {
            delete cache[i];
        }
    }
}

Block* BlockCache::AllocateBlock(u32 address) {
    // first see if we need to allocate a block page
    // get the top 13 bits of the address to determine the page index
    int page_index = address >> 19;
    BlockPage* block_page = cache[page_index];

    if (block_page == nullptr) {
        cache[page_index] = new BlockPage;
    }

    // now we must look determine the block index in the selected page
    int block_index = page_index / 128;
    cache[page_index]->page[block_index] = new Block;
    return cache[page_index]->page[block_index];
}