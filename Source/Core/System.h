#pragma once

#include "Core/ARM/cpu_core.h"
#include "Core/ARM/ARM7/Memory.h"
#include "Core/ARM/ARM9/Memory.h"
#include "Core/Scheduler.h"
#include "Core/Hardware/Cartridge/Cartridge.h"
#include "Core/Hardware/SPI.h"
#include "Core/Hardware/cp15/cp15.h"
#include "Core/Hardware/DMA.h"
#include "Core/Hardware/Input.h"
#include "Core/Hardware/IPC.h"
#include "Core/Hardware/Timers.h"
#include "Core/Hardware/SPU.h"
#include "Core/Hardware/RTC.h"
#include "Core/Hardware/MathsUnit.h"
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

    void write_haltcnt(u8 data);
    void write_wramcnt(u8 data);
    
    bool CartridgeAccessRights();
    void SetCPUCoreType(CPUCoreType type);
    std::string GetCPUCoreType();

    VideoUnit video_unit;
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

    CPUCore cpu_core[2];

    u8 main_memory[0x400000] = {};
    u8 shared_wram[0x8000] = {};

    u8 wramcnt;
    u8 powcnt2;
    u16 rcnt;
    u8 HALTCNT;
    u16 exmemcnt;
    u8 postflg7;
    u8 postflg9;
    u32 biosprot;
    u16 siocnt;

    std::string rom_path;
    CPUCoreType core_type;
};
