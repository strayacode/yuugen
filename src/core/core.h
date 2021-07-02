#pragma once

#include <core/arm/memory.h>
#include <core/arm/arm7.h>
#include <core/arm/arm9.h>
#include <core/scheduler/scheduler.h>
#include <core/hw/cartridge/cartridge.h>
#include <core/hw/spi/spi.h>
#include <core/hw/cp15/cp15.h>
#include <core/hw/gpu/gpu.h>
#include <core/hw/dma/dma.h>
#include <core/hw/input/input.h>
#include <core/hw/ipc/ipc.h>
#include <core/hw/interrupt/interrupt.h>
#include <core/hw/timers/timers.h>
#include <core/hw/spu/spu.h>
#include <core/hw/rtc/rtc.h>
#include <core/hw/maths_unit/maths_unit.h>
#include <core/hw/wifi/wifi.h>
#include <string>
#include <algorithm>

struct Core {
    Core();
    void Reset();
    void DirectBoot();
    void FirmwareBoot();

    void SetRomPath(std::string path);

    void RunFrame();

    Cartridge cartridge;

    Memory memory;
    ARM7 arm7;
    ARM9 arm9;

    Scheduler scheduler;

    SPI spi;

    CP15 cp15;

    GPU gpu;

    DMA dma[2];

    Input input;

    IPC ipc;

    Interrupt interrupt[2];

    Timers timers[2];

    SPU spu;

    RTC rtc;

    MathsUnit maths_unit;

    Wifi wifi;

    std::string rom_path;
};