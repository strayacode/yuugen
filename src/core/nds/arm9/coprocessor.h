#pragma once

#include "common/types.h"
#include "core/arm/coprocessor.h"

namespace core::nds {

class ARM9Memory;

class ARM9Coprocessor : public arm::Coprocessor {
public:
    ARM9Coprocessor(ARM9Memory& memory);

    u32 read(u32 cn, u32 cm, u32 cp) override;
    void write(u32 cn, u32 cm, u32 cp, u32 value) override;

private:
    ARM9Memory& memory;
};

} // namespace core::nds