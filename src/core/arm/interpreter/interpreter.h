#pragma once

#include "core/arm/cpu.h"

namespace core::arm {

class Interpreter : public CPU {
public:
    void run(int cycles) override;

private:
};

} // namespace core::arm