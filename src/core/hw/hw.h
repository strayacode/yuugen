#pragma once

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
#include <common/types.h>
#include <string>
#include <algorithm>
#include <array>

enum MemoryRegion {
    REGION_ARM7_BIOS = 0x00,
    REGION_MAIN_MEMORY = 0x02,
    REGION_SHARED_WRAM = 0x03,
    REGION_IO = 0x04,
    REGION_PALETTE_RAM = 0x05,
    REGION_VRAM = 0x06,
    REGION_OAM = 0x07,
    REGION_GBA_ROM_L = 0x08,
    REGION_GBA_ROM_H = 0x09,
    REGION_GBA_RAM = 0x0A,
    REGION_ARM9_BIOS = 0xFF,
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

    // regular memory read/write handlers
    template <typename T>
    auto ARM7Read(u32 addr) -> T;

    template <typename T>
    void ARM7Write(u32 addr, T data);

    template <typename T>
    auto ARM9Read(u32 addr) -> T;

    template <typename T>
    void ARM9Write(u32 addr, T data);

    // software fastmem read/write handlers
    template <typename T>
    auto ARM7FastRead(u32 addr) -> T;

    template <typename T>
    void ARM7FastWrite(u32 addr, T data);

    template <typename T>
    auto ARM9FastRead(u32 addr) -> T;

    template <typename T>
    void ARM9FastWrite(u32 addr, T data);

    // mmio handlers
    auto ARM7ReadByteIO(u32 addr) -> u8;
    auto ARM7ReadHalfIO(u32 addr) -> u16;
    auto ARM7ReadWordIO(u32 addr) -> u32;

    void ARM7WriteByteIO(u32 addr, u8 data);
    void ARM7WriteHalfIO(u32 addr, u16 data);
    void ARM7WriteWordIO(u32 addr, u32 data);

    auto ARM9ReadByteIO(u32 addr) -> u8;
    auto ARM9ReadHalfIO(u32 addr) -> u16;
    auto ARM9ReadWordIO(u32 addr) -> u32;

    void ARM9WriteByteIO(u32 addr, u8 data);
    void ARM9WriteHalfIO(u32 addr, u16 data);
    void ARM9WriteWordIO(u32 addr, u32 data);

    void WriteHALTCNT(u8 data);

    void WriteWRAMCNT(u8 data);

    auto CartridgeAccessRights() -> bool;

    void MapSharedMemory(u8 data);

    void UpdateARM9MemoryMap(u32 low_addr, u32 high_addr);
    void UpdateARM7MemoryMap(u32 low_addr, u32 high_addr);

    void LoadARM7Bios();
    void LoadARM9Bios();

    Cartridge cartridge;
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
private:
    // todo: maybe make vectors later or std::array?
    u8 main_memory[0x400000] = {};
    u8 arm7_wram[0x10000] = {};
    u8 shared_wram[0x8000] = {};

    std::vector<u8> arm7_bios;
    std::vector<u8> arm9_bios;

    // used by the arm7 and arm9 to configure mapping of shared wram
    // but is only on the arm9 bus
    u8 WRAMCNT;
    u8 POWCNT2;
    u16 RCNT;

    // used to halt the arm7
    u8 HALTCNT;

    u16 EXMEMCNT;

    u8 POSTFLG7;
    u8 POSTFLG9;

    // used for protection of the arm7 bios
    u32 BIOSPROT;

    // the arm7 provides io ports for the link port but it doesn't seem to be used
    u16 SIOCNT;

    std::array<u8*, 0x100000> arm7_read_page_table;
    std::array<u8*, 0x100000> arm9_read_page_table;
    std::array<u8*, 0x100000> arm7_write_page_table;
    std::array<u8*, 0x100000> arm9_write_page_table;

    std::string rom_path;
};