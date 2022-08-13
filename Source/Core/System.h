#pragma once

#include "Core/arm/cpu_core.h"
#include "Core/arm/arm7/memory.h"
#include "Core/arm/arm9/memory.h"
#include "Core/Scheduler.h"
#include "Core/HW/cartridge/cartridge.h"
#include "Core/HW/spi/spi.h"
#include "Core/HW/cp15/cp15.h"
#include "Core/HW/dma/dma.h"
#include "Core/HW/input/input.h"
#include "Core/HW/ipc/ipc.h"
#include "Core/HW/timers/timers.h"
#include "Core/HW/spu/spu.h"
#include "Core/HW/rtc/rtc.h"
#include "Core/HW/maths_unit/maths_unit.h"
#include "Core/HW/wifi/wifi.h"
#include "VideoCommon/VideoUnit.h"

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
    void write_wramcnt(u8 data);
    bool CartridgeAccessRights();
    void SetCPUCoreType(CPUCoreType type);
    std::string GetCPUCoreType();

    ARM7Memory arm7_memory;
    ARM9Memory arm9_memory;

    Scheduler scheduler;
    Cartridge cartridge;
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
    VideoUnit video_unit;

    CPUCore cpu_core[2];

    u8 main_memory[0x400000] = {};
    u8 shared_wram[0x8000] = {};

    u8 wramcnt;
    u8 POWCNT2;
    u16 RCNT;
    u8 HALTCNT;
    u16 EXMEMCNT;
    u8 POSTFLG7;
    u8 postflg9;
    u32 BIOSPROT;
    u16 SIOCNT;

    std::string rom_path;
    CPUCoreType core_type;
};