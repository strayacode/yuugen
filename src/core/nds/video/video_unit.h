#pragma once

#include "core/nds/video/vram.h"

namespace core::nds {

class VideoUnit {
public:
    void reset();

    VRAM vram;
    
private:
};

} // namespace core::nds