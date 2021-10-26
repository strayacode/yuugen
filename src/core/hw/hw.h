#pragma once

#include <core/arm/interpreter/interpreter.h>
#include <core/arm/arm7/memory.h>
#include <core/arm/arm9/memory.h>
#include <core/scheduler/scheduler.h>
#include <core/hw/cartridge/cartridge.h>
#include <core/hw/spi/spi.h>
#include <core/hw/cp15/cp15.h>
#include <core/hw/gpu/gpu.h>
#include <core/hw/dma/dma.h>
#include <core/hw/input/input.h>
#include <core/hw/ipc/ipc.h>
#include <core/hw/timers/timers.h>
#include <core/hw/spu/spu.h>
#include <core/hw/rtc/rtc.h>
#include <core/hw/maths_unit/maths_unit.h>
#include <core/hw/wifi/wifi.h>
#include <common/types.h>
#include <string>
#include <algorithm>
#include <array>
#include <memory>

enum class CPUCoreType {
    Interpreter,
};

// this class contains all the hardware of the ds
class HW {
public:
    HW();
    ~HW();
    void Reset();
    void DirectBoot();
    void FirmwareBoot();
    void RunFrame();
    void SetRomPath(std::string path);
    void WriteHALTCNT(u8 data);
    void WriteWRAMCNT(u8 data);
    bool CartridgeAccessRights();
    void LoadARM7Bios();
    void LoadARM9Bios();
    void InitialiseCPUCores(CPUCoreType core_type);

    Cartridge cartridge;
    Scheduler scheduler;
    SPI spi;
    CP15 cp15;
    GPU gpu;
    DMA dma[2];
    Input input;
    IPC ipc;
    Timers timers[2];
    SPU spu;
    RTC rtc;
    MathsUnit maths_unit;
    Wifi wifi;
    ARM7Memory arm7_memory;
    ARM9Memory arm9_memory;

    std::unique_ptr<CPUBase> cpu_core[2];

    u8 main_memory[0x400000] = {};
    u8 arm7_wram[0x10000] = {};
    u8 shared_wram[0x8000] = {};

    std::vector<u8> arm7_bios;
    std::vector<u8> arm9_bios;

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
};