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

enum class CPUCoreType {
    Interpreter,
};

struct System {
public:
    System() : cartridge(*this),
        spi(*this),
        cp15(*this),
        gpu(*this),
        dma {DMA(*this, 0), DMA(*this, 1)},
        ipc(*this),
        timers {Timers(*this, 0), Timers(*this, 1)},
        spu(*this),
        rtc(*this),
        arm7_memory(*this),
        arm9_memory(*this) {
        InitialiseCPUCores(CPUCoreType::Interpreter);
    }

    ~System() {

    }

    void Reset() {
        LoadARM7Bios();
        LoadARM9Bios();
        InitialiseCPUCores(CPUCoreType::Interpreter);

        arm7_memory.Reset();
        arm9_memory.Reset();
        scheduler.Reset();
        cartridge.Reset();
        cartridge.LoadRom(rom_path);
        gpu.Reset();
        dma[0].Reset();
        dma[1].Reset();
        timers[0].Reset();
        timers[1].Reset();
        cp15.Reset();
        spi.Reset();
        input.Reset();
        ipc.Reset();
        maths_unit.Reset();
        wifi.Reset();

        memset(main_memory, 0, 0x400000);
        memset(arm7_wram, 0, 0x10000);
        memset(shared_wram, 0, 0x8000);
        
        arm7_bios.clear();
        arm9_bios.clear();

        WRAMCNT = 0;
        POWCNT2 = 0;
        RCNT = 0;
        HALTCNT = 0;
        EXMEMCNT = 0;
        POSTFLG7 = 0;
        POSTFLG9 = 0;
        BIOSPROT = 0;
        SIOCNT = 0;

        cpu_core[0]->Reset();
        cpu_core[1]->Reset();
    }

    void DirectBoot() {
        cp15.DirectBoot();

        RCNT = 0x8000;

        arm9_memory.FastWrite<u8>(0x4000247, 0x03); // WRAMCNT
        arm9_memory.FastWrite<u8>(0x4000300, 0x01); // POSTFLG (ARM9)
        arm7_memory.FastWrite<u8>(0x4000300, 0x01); // POSTFLG (ARM7)
        arm9_memory.FastWrite<u16>(0x4000304, 0x0001); // POWCNT1
        arm7_memory.FastWrite<u16>(0x4000504, 0x0200); // SOUNDBIAS

        // Set some memory values as the BIOS/firmware would
        arm9_memory.FastWrite<u32>(0x27FF800, 0x00001FC2); // Chip ID 1
        arm9_memory.FastWrite<u32>(0x27FF804, 0x00001FC2); // Chip ID 2
        arm9_memory.FastWrite<u16>(0x27FF850, 0x5835); // ARM7 BIOS CRC
        arm9_memory.FastWrite<u16>(0x27FF880, 0x0007); // Message from ARM9 to ARM7
        arm9_memory.FastWrite<u16>(0x27FF884, 0x0006); // ARM7 boot task
        arm9_memory.FastWrite<u32>(0x27FFC00, 0x00001FC2); // Copy of chip ID 1
        arm9_memory.FastWrite<u32>(0x27FFC04, 0x00001FC2); // Copy of chip ID 2
        arm9_memory.FastWrite<u16>(0x27FFC10, 0x5835); // Copy of ARM7 BIOS CRC
        arm9_memory.FastWrite<u16>(0x27FFC40, 0x0001); // Boot indicator

        cartridge.DirectBoot();
        cpu_core[0]->DirectBoot(cartridge.header.arm7_entrypoint);
        cpu_core[1]->DirectBoot(cartridge.header.arm9_entrypoint);    
        spi.DirectBoot();
    }

    void FirmwareBoot() {
        cpu_core[0]->FirmwareBoot();
        cpu_core[1]->FirmwareBoot();
        cartridge.FirmwareBoot();
    }

    void SetRomPath(std::string path) {
        rom_path = path;
    }

    void RunFrame() {
        u64 frame_end_time = scheduler.GetCurrentTime() + 560190;

        while (scheduler.GetCurrentTime() < frame_end_time) {
            cpu_core[1]->Run(2);
            cpu_core[0]->Run(1);
            
            scheduler.Tick(1);
            scheduler.RunEvents();
        }
    }

    void WriteHALTCNT(u8 data) {
        HALTCNT = data & 0xC0;

        u8 power_down_mode = (HALTCNT >> 6) & 0x3;

        // check bits 6..7 to see what to do
        switch (power_down_mode) {
        case 2:
            cpu_core[0]->Halt();
            break;
        case 3:
            log_warn("unhandled request for sleep mode");
            break;
        default:
            log_fatal("power down mode %d is not implemented!", power_down_mode);
            break;
        }
    }

    void WriteWRAMCNT(u8 data) {
        WRAMCNT = data & 0x3;

        // now we must update the memory map for the shared wram space specifically
        arm7_memory.UpdateMemoryMap(0x03000000, 0x04000000);
        arm9_memory.UpdateMemoryMap(0x03000000, 0x04000000);
    }

    bool CartridgeAccessRights() {
        // check which cpu has access to the nds cartridge
        if (EXMEMCNT & (1 << 11)) {
            return false; // 0 = ARMv4
        } else {
            return true; // 1 = ARMv5
        }
    }

    void LoadARM7Bios() {
        std::ifstream file("../bios/bios7.bin", std::ios::binary);

        if (!file) {
            log_fatal("[Memory] ARM7 bios could not be found");
        }

        file.unsetf(std::ios::skipws);

        std::streampos size;

        file.seekg(0, std::ios::beg);

        arm7_bios.reserve(0x4000);
        arm7_bios.insert(arm7_bios.begin(), std::istream_iterator<u8>(file), std::istream_iterator<u8>());
        file.close();

        log_debug("[Memory] ARM7 bios loaded successfully!");
    }

    void LoadARM9Bios() {
        std::ifstream file("../bios/bios9.bin", std::ios::binary);

        if (!file) {
            log_fatal("[Memory] ARM9 bios could not be found");
        }

        file.unsetf(std::ios::skipws);

        std::streampos size;

        file.seekg(0, std::ios::beg);

        arm9_bios.reserve(0x8000);
        arm9_bios.insert(arm9_bios.begin(), std::istream_iterator<u8>(file), std::istream_iterator<u8>());
        file.close();

        log_debug("[Memory] ARM9 bios loaded successfully!");
    }

    void InitialiseCPUCores(CPUCoreType core_type) {
        if (core_type == CPUCoreType::Interpreter) {
            cpu_core[0] = std::make_unique<Interpreter>(arm7_memory, CPUArch::ARMv4, nullptr);
            cpu_core[1] = std::make_unique<Interpreter>(arm9_memory, CPUArch::ARMv5, &cp15);
        } else {
            log_fatal("handle different cpu core!");
        }
    }

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

    // TODO: combine arm7 and arm9 memory into singular memory class including the shared memory
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