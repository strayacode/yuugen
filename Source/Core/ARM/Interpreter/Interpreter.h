#pragma once

#include "Common/Types.h"
#include "Core/ARM/ExecutorInterface.h"
#include "Core/ARM/CPU.h"

class Interpreter final : public ExecutorInterface {
public:
    Interpreter(CPU& cpu);

    void run(u64 target) override;

private:
    template <typename T>
    T read(u32 addr);

    template <typename T>
    void write(u32 addr, T data);

    CPU& m_cpu;
};

