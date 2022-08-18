#include <algorithm>
#include "Core/System.h"

System::System() 
    : arm7_memory(*this), arm9_memory(*this), cartridge(*this),
    spi(*this), cp15(*this), 
    dma {DMA(*this, 0), DMA(*this, 1)}, ipc(*this), 
    timers {Timers(*this, 0), Timers(*this, 1)},
    spu(*this), video_unit(*this),
    cpu_core {CPUCore(arm7_memory, Arch::ARMv4, nullptr), CPUCore(arm9_memory, Arch::ARMv5, &cp15)} {
    SetCPUCoreType(CPUCoreType::Interpreter);
}

void System::Reset() {
    SetCPUCoreType(CPUCoreType::Interpreter);
    
    arm7_memory.reset();
    arm9_memory.reset();
    scheduler.Reset();
    cartridge.Reset();
    cartridge.LoadRom(rom_path);
    video_unit.reset();
    dma[0].reset();
    dma[1].reset();
    timers[0].reset();
    timers[1].reset();
    cp15.Reset();
    spi.Reset();
    input.reset();
    ipc.reset();
    maths_unit.reset();
    wifi.Reset();
    spu.reset();
    rtc.Reset();

    memset(main_memory, 0, 0x400000);
    memset(shared_wram, 0, 0x8000);

    wramcnt = 0;
    POWCNT2 = 0;
    RCNT = 0;
    HALTCNT = 0;
    exmemcnt = 0;
    postflg7 = 0;
    postflg9 = 0;
    BIOSPROT = 0;
    SIOCNT = 0;

    cpu_core[0].Reset();
    cpu_core[1].Reset();
}

void System::DirectBoot() {
    cp15.DirectBoot();

    RCNT = 0x8000;

    arm9_memory.write<u8>(0x4000247, 0x03); // wramcnt
    arm9_memory.write<u8>(0x4000300, 0x01); // POSTFLG (ARM9)
    arm7_memory.write<u8>(0x4000300, 0x01); // POSTFLG (ARM7)
    arm9_memory.write<u16>(0x4000304, 0x0001); // POWCNT1
    arm7_memory.write<u16>(0x4000504, 0x0200); // SOUNDBIAS
    arm9_memory.write<u32>(0x27FF800, 0x00001FC2); // Chip ID 1
    arm9_memory.write<u32>(0x27FF804, 0x00001FC2); // Chip ID 2
    arm9_memory.write<u16>(0x27FF850, 0x5835); // ARM7 BIOS CRC
    arm9_memory.write<u16>(0x27FF880, 0x0007); // Message from ARM9 to ARM7
    arm9_memory.write<u16>(0x27FF884, 0x0006); // ARM7 boot task
    arm9_memory.write<u32>(0x27FFC00, 0x00001FC2); // Copy of chip ID 1
    arm9_memory.write<u32>(0x27FFC04, 0x00001FC2); // Copy of chip ID 2
    arm9_memory.write<u16>(0x27FFC10, 0x5835); // Copy of ARM7 BIOS CRC
    arm9_memory.write<u16>(0x27FFC40, 0x0001); // Boot indicator

    cartridge.DirectBoot();
    cpu_core[0].DirectBoot(cartridge.loader.GetARM7Entrypoint());
    cpu_core[1].DirectBoot(cartridge.loader.GetARM9Entrypoint());    
    spi.DirectBoot();
}

void System::FirmwareBoot() {
    cpu_core[0].FirmwareBoot();
    cpu_core[1].FirmwareBoot();
    cartridge.FirmwareBoot();
}

void System::SetGamePath(std::string path) {
    rom_path = path;
}

void System::RunFrame() {
    u64 frame_end_time = scheduler.GetCurrentTime() + 560190;

    while (scheduler.GetCurrentTime() < frame_end_time) {
        if (!cpu_core[0].Halted() || !cpu_core[1].Halted()) {
            u64 cycles = std::min(static_cast<u64>(16), scheduler.GetEventTime() - scheduler.GetCurrentTime());
            u64 arm7_target = scheduler.GetCurrentTime() + cycles;
            u64 arm9_target = scheduler.GetCurrentTime() + (cycles << 1);
            
            // run the arm9 until the next scheduled event
            cpu_core[1].run(arm9_target);

            // let the arm7 catch up
            cpu_core[0].run(arm7_target);

            // advance the scheduler
            scheduler.Tick(cycles);
        } else {
            // if both cpus are halted we can just advance to the next event
            scheduler.set_current_time(scheduler.GetEventTime());
        }

        scheduler.RunEvents();
    }
}

void System::WriteHALTCNT(u8 data) {
    HALTCNT = data & 0xC0;

    u8 power_down_mode = (HALTCNT >> 6) & 0x3;

    // check bits 6..7 to see what to do
    switch (power_down_mode) {
    case 2:
        cpu_core[0].Halt();
        break;
    case 3:
        log_warn("unhandled request for sleep mode");
        break;
    default:
        log_fatal("power down mode %d is not implemented!", power_down_mode);
        break;
    }
}

void System::write_wramcnt(u8 data) {
    wramcnt = data & 0x3;

    // now we must update the memory map for the shared wram space specifically
    arm7_memory.update_memory_map(0x03000000, 0x04000000);
    arm9_memory.update_memory_map(0x03000000, 0x04000000);
}

bool System::CartridgeAccessRights() {
    // check which cpu has access to the nds cartridge
    if (exmemcnt & (1 << 11)) {
        return false; // 0 = ARMv4
    } else {
        return true; // 1 = ARMv5
    }
}

void System::SetCPUCoreType(CPUCoreType type) {
    core_type = type;
}

std::string System::GetCPUCoreType() {
    return "Interpreter";
}