#pragma once

#include "Common/Types.h"
#include "Core/ARM/ExecutorInterface.h"
#include "Core/ARM/CPU.h"

class Interpreter final : public ExecutorInterface {
public:
    Interpreter(CPU& cpu);

    void run(u64 target) override;

private:
    CPU& cpu;
};

