#pragma once

#include <unordered_map>
#include <memory>
#include "common/logger.h"
#include "arm/jit/basic_block.h"

namespace arm {

class BlockCache {
public:
    void reset() {
        block_map.clear();
    }

    BasicBlock* get(BasicBlock::Key key) {
        return block_map[key.value].get();
    }

    void set(BasicBlock::Key key, BasicBlock* basic_block) {
        block_map[key.value] = std::unique_ptr<BasicBlock>{basic_block};
    }

private:
    // maps a starting pc to a BasicBlock
    std::unordered_map<u64, std::unique_ptr<BasicBlock>> block_map;
};

} // namespace arm