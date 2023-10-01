#pragma once

#include "common/types.h"
#include "arm/coprocessor.h"

namespace nds {

class ARM7Coprocessor : public arm::Coprocessor {
public:
    void reset() override;
    u32 read(u32 cn, u32 cm, u32 cp) override;
    void write(u32 cn, u32 cm, u32 cp, u32 value) override;
    u32 get_exception_base() override;
};

} // namespace nds