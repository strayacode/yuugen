#pragma once

namespace arm {

struct Backend {
    virtual ~Backend() = default;

    virtual Code compile(BasicBlock& basic_block) = 0;
};

} // namespace arm