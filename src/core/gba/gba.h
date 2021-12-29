#pragma once

#include "core/system.h"
#include "core/arm/cpu_core.h"
#include "core/gba/memory/memory.h"
#include "core/gba/cartridge/cartridge.h"
#include "core/gba/gpu/gpu.h"
#include "core/scheduler/scheduler.h"

class GBA : public System {
public:
    GBA();

    std::string GetSystem() override {
        return "Game Boy Advance";
    }

    void Reset() override;
    

    // TODO: use a struct called BootParameters instead when we decide to add more
    // parameters
    void Boot(bool direct) override;
    void RunFrame() override;
    const u32* GetFramebuffer(int screen) override;

    GBAMemory memory;
    GBACartridge cartridge;
    CPUCore cpu_core;
    Scheduler scheduler;
    GBAGPU gpu;
};