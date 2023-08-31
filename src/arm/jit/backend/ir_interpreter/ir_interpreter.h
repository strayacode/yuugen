#pragma once

#include <vector>
#include "arm/jit/backend/backend.h"
#include "arm/jit/backend/code_cache.h"

namespace arm {

class IRInterpreter : public Backend {
public:
    virtual void reset() override;
    virtual bool has_code_at(Location location) override;
    virtual void compile(BasicBlock& basic_block) override;
    virtual void run(Location location) override;

private:
    using CachedFunction = void (IRInterpreter::*)();

    using CachedFunctions = std::vector<CachedFunction>;

    CodeCache<CachedFunctions> code_cache;
};

} // namespace arm