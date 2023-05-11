#pragma once

#include "common/types.h"

namespace common {

struct VideoDevice {
    virtual ~VideoDevice() = default;
    virtual void update_texture(u32* pointer) = 0;
    virtual void destroy() = 0;
};

} // namespace common