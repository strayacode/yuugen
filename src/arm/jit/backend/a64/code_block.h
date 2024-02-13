#pragma once

#include "common/types.h"
#include "common/platform.h"
#include "common/logger.h"

#if defined(PLATFORM_OSX)
#include <sys/mman.h>
#include <errno.h>
#include <pthread.h>
#include <libkern/OSCacheControl.h>
#define MAP_ANONYMOUS MAP_ANON
#endif

namespace arm {

class CodeBlock {
public:
    CodeBlock(u64 capacity) : capacity(capacity) {
#if defined(PLATFORM_OSX)
        code = reinterpret_cast<u32*>(mmap(nullptr, capacity, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE | MAP_JIT, -1, 0));
#endif

        if (code == nullptr || code == reinterpret_cast<void*>(-1)) {
            LOG_ERROR("CodeBlock: error allocating");
        } else {
            LOG_DEBUG("CodeBlock: successfully allocated");
        }
    }

    ~CodeBlock() {
        if (munmap(code, capacity) == -1) {
            LOG_ERROR("CodeBlock: error deallocating");
        } else {
            LOG_DEBUG("CodeBlock: successfully deallocated");
        }
    }

    void unprotect() {
#if defined(PLATFORM_OSX)
        pthread_jit_write_protect_np(false);
#endif
    }

    void protect() {
#if defined(PLATFORM_OSX)
        pthread_jit_write_protect_np(true);
#endif
    }

    void invalidate(u32* start, u64 size) {
#if defined(PLATFORM_OSX)
        sys_icache_invalidate(start, size);
#endif
    }

    void invalidate_all() {
#if defined(PLATFORM_OSX)
        sys_icache_invalidate(code, capacity);
#endif
    }

    u32* get_code() const { return code; }

private:
    u32* code;
    u64 capacity;
};

} // namespace arm