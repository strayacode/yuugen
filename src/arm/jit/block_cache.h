#pragma once

#include <unordered_map>
#include "arm/jit/basic_block.h"

namespace arm {

class BlockCache {
public:
    void reset() {
        block_map.clear();
    }

    BasicBlock* lookup(u32 pc) {
        return &block_map[pc];
    }

private:
    // maps a starting pc to a BasicBlock
    std::unordered_map<u32, BasicBlock> block_map;
};

} // namespace arm