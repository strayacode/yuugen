#pragma once

#include "Common/Types.h"
#include "Core/ARM/CPUBase.h"

class System;

class Interpreter final : public CPUBase {
public:
    Interpreter(MemoryBase& memory, CoprocessorBase& coprocessor, Arch arch);

    void run(u64 target) override;

private:
    template <typename T>
    T read(u32 addr);

    template <typename T>
    void write(u32 addr, T data);

    void arm_flush_pipeline() override;
    void thumb_flush_pipeline() override;

    std::array<u32, 2> m_pipeline;
};

