#pragma once

#include "common/types.h"
#include "common/platform.h"
#include "common/logger.h"

#if defined(PLATFORM_OSX)
#include <sys/mman.h>
#include <errno.h>
#define MAP_ANONYMOUS MAP_ANON
#endif

namespace arm {

class CodeBlock {
public:
    CodeBlock(int size) : size(size) {
#if defined(PLATFORM_OSX)
        code = reinterpret_cast<u32*>(mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE | MAP_JIT, -1, 0));
#endif

        if (code == nullptr || code == reinterpret_cast<void*>(-1)) {
            logger.error("CodeBlock: error allocating");
        } else {
            logger.debug("CodeBlock: successfully allocated");
        }
    }

    ~CodeBlock() {
        if (munmap(code, size) == -1) {
            logger.error("CodeBlock: error deallocating");
        } else {
            logger.debug("CodeBlock: successfully deallocated");
        }
    }

    u32* get_code() const { return code; }

private:
    u32* code;
    int size;
};

} // namespace arm