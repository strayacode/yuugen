#pragma once

#include <array>
#include "Core/ARM/ARM7/ARM7.h"
#include "Core/ARM/ARM9/ARM9.h"
#include "Core/Scheduler.h"
#include "Core/Hardware/Cartridge/Cartridge.h"
#include "Core/Hardware/SPI.h"
#include "Core/Hardware/DMA.h"
#include "Core/Hardware/Input.h"
#include "Core/Hardware/IPC.h"
#include "Core/Hardware/Timers.h"
#include "Core/Hardware/SPU.h"
#include "Core/Hardware/RTC.h"
#include "Core/Hardware/MathsUnit.h"
#include "VideoCommon/VideoUnit.h"

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

    // get a reference to a cpu based on an id
    // (0 = arm7, 1 = arm9)
    inline CPUBase& cpu(int cpu) {
        return cpu ? arm9.cpu() : arm7.cpu();
    }
    
    VideoUnit video_unit;

    Scheduler scheduler;
    Cartridge cartridge;
    SPI spi;
    DMA dma[2];
    Input input;
    IPC ipc;
    Timers timers[2];
    SPU spu;
    RTC rtc;
    MathsUnit maths_unit;

    ARM7 arm7;
    ARM9 arm9;

    std::array<u8, 0x400000> main_memory;
    std::array<u8, 0x8000> shared_wram;

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
};
