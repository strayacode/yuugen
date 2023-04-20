#pragma once

#include "common/types.h"
#include "core/arm/coprocessor.h"

namespace core::nds {

class ARM7Coprocessor : public arm::Coprocessor {
public:
    u32 read(u32 cn, u32 cm, u32 cp) override;
    void write(u32 cn, u32 cm, u32 cp, u32 value) override;
    u32 get_exception_base() override;
};

} // namespace core::nds