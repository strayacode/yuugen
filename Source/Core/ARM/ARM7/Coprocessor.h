#pragma once

#include "Core/ARM/CoprocessorBase.h"

class ARM7Coprocessor : public CoprocessorBase {
public:
    ARM7Coprocessor() = default;

    u32 get_exception_base() override;

    u32 read(u32 cn, u32 cm, u32 cp) override;
    void write(u32 cn, u32 cm, u32 cp, u32 data) override;
};
