#pragma once

#include "Core/arm/cpu_core.h"
#include "Core/arm/arm7/memory.h"
#include "Core/arm/arm9/memory.h"
#include "Core/scheduler/scheduler.h"
#include "Core/hw/cartridge/cartridge.h"
#include "Core/hw/spi/spi.h"
#include "Core/hw/cp15/cp15.h"
#include "Core/hw/dma/dma.h"
#include "Core/hw/input/input.h"
#include "Core/hw/ipc/ipc.h"
#include "Core/hw/timers/timers.h"
#include "Core/hw/spu/spu.h"
#include "Core/hw/rtc/rtc.h"
#include "Core/hw/maths_unit/maths_unit.h"
#include "Core/hw/wifi/wifi.h"
#include "VideoCommon/GPU.h"

enum class CPUCoreType {
    Interpreter,
};

class System {
public:
    System();

    void Reset();
    void DirectBoot();
    void FirmwareBoot();
    void SetGamePath(std::string path);
    void RunFrame();
    void WriteHALTCNT(u8 data);
    void WriteWRAMCNT(u8 data);
    bool CartridgeAccessRights();
    void SetCPUCoreType(CPUCoreType type);
    std::string GetCPUCoreType();

    Cartridge cartridge;
    Scheduler scheduler;
    SPI spi;
    CP15 cp15;
    DMA dma[2];
    Input input;
    IPC ipc;
    Timers timers[2];
    SPU spu;
    RTC rtc;
    MathsUnit maths_unit;
    Wifi wifi;
    GPU gpu;

    // TODO: combine arm7 and arm9 memory into singular memory class including the shared memory
    ARM7Memory arm7_memory;
    ARM9Memory arm9_memory;

    CPUCore cpu_core[2];

    u8 main_memory[0x400000] = {};
    u8 shared_wram[0x8000] = {};

    u8 WRAMCNT;
    u8 POWCNT2;
    u16 RCNT;
    u8 HALTCNT;
    u16 EXMEMCNT;
    u8 POSTFLG7;
    u8 POSTFLG9;
    u32 BIOSPROT;
    u16 SIOCNT;

    std::string rom_path;
    CPUCoreType core_type;
};