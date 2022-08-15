#pragma once

#include "Core/ARM/cpu_core.h"
#include "Core/ARM/ARM7/Memory.h"
#include "Core/ARM/ARM9/Memory.h"
#include "Core/Scheduler.h"
#include "Core/Hardware/Cartridge/cartridge.h"
#include "Core/Hardware/spi/spi.h"
#include "Core/Hardware/cp15/cp15.h"
#include "Core/Hardware/dma/dma.h"
#include "Core/Hardware/Input.h"
#include "Core/Hardware/IPC.h"
#include "Core/Hardware/timers/timers.h"
#include "Core/Hardware/spu/spu.h"
#include "Core/Hardware/rtc/rtc.h"
#include "Core/Hardware/MathsUnit.h"
#include "Core/Hardware/wifi/wifi.h"
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
    u16 exmemcnt;
    u8 postflg7;
    u8 postflg9;
    u32 BIOSPROT;
    u16 SIOCNT;

    std::string rom_path;
    CPUCoreType core_type;
};