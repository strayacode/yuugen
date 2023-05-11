#pragma once

#include "common/types.h"

namespace arm {

struct Coprocessor {
    virtual ~Coprocessor() = default;
    
    virtual void reset() = 0;
    virtual u32 read(u32 cn, u32 cm, u32 cp) = 0;
    virtual void write(u32 cn, u32 cm, u32 cp, u32 value) = 0;
    virtual u32 get_exception_base() = 0;

    struct TCM {
        u8* data = nullptr;
        u32 mask = 0;

        struct Config {
            bool enable_reads = false;
            bool enable_writes = false;
            u32 base = 0;
            u32 limit = 0;
        } config;
    };
};

} // namespace arm