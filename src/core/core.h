#pragma once

#include <memory>
#include <core/arm.h>
#include <core/memory.h>
#include <core/cartridge.h>
#include <core/gpu.h>
#include <core/dma.h>
#include <core/cp15.h>
#include <core/input.h>
#include <core/interrupt.h>
#include <core/ipc.h>
#include <core/timers.h>
#include <core/rtc.h>
#include <core/spu.h>
#include <core/spi.h>
#include <core/maths_unit.h>
#include <string.h>

struct Core {
    Core();
    void Reset();
    void DirectBoot();
    void FirmwareBoot();
    void RunFrame();
    void SetRomPath(const char* path);

    

    
    Cartridge cartridge;
    
    ARM arm9;
    ARM arm7;

    GPU gpu;

    DMA dma[2];

    CP15 cp15;

    Input input;

    Interrupt interrupt[2];

    IPC ipc;

    Timers timers[2];

    RTC rtc;

    SPU spu;

    SPI spi;

    Memory memory;

    MathsUnit maths_unit;

    const char* rom_path = nullptr;
};
