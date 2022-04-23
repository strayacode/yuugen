#include "Core/system.h"

System::System() 
    : cartridge(*this), spi(*this), cp15(*this),
    dma {DMA(*this, 0), DMA(*this, 1)}, 
    ipc(*this), timers {Timers(*this, 0), Timers(*this, 1)}, 
    spu(*this), gpu(*this), arm7_memory(*this), arm9_memory(*this),
    cpu_core {CPUCore(arm7_memory, CPUArch::ARMv4, nullptr), CPUCore(arm9_memory, CPUArch::ARMv5, &cp15)} {
    SetCPUCoreType(CPUCoreType::Interpreter);
}

void System::Reset() {
    SetCPUCoreType(CPUCoreType::Interpreter);
    gpu.create_renderers(RendererType::Software);

    arm7_memory.Reset();
    arm9_memory.Reset();
    scheduler.Reset();
    cartridge.Reset();
    cartridge.LoadRom(rom_path);
    gpu.reset();
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
    memset(shared_wram, 0, 0x8000);

    wramcnt = 0;
    POWCNT2 = 0;
    RCNT = 0;
    HALTCNT = 0;
    EXMEMCNT = 0;
    POSTFLG7 = 0;
    POSTFLG9 = 0;
    BIOSPROT = 0;
    SIOCNT = 0;

    cpu_core[0].Reset();
    cpu_core[1].Reset();
}

void System::DirectBoot() {
    cp15.DirectBoot();

    RCNT = 0x8000;

    arm9_memory.FastWrite<u8>(0x4000247, 0x03); // wramcnt
    arm9_memory.FastWrite<u8>(0x4000300, 0x01); // POSTFLG (ARM9)
    arm7_memory.FastWrite<u8>(0x4000300, 0x01); // POSTFLG (ARM7)
    arm9_memory.FastWrite<u16>(0x4000304, 0x0001); // POWCNT1
    arm7_memory.FastWrite<u16>(0x4000504, 0x0200); // SOUNDBIAS
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
        cpu_core[1].RunInterpreter(2);
        cpu_core[0].RunInterpreter(1);
        
        scheduler.Tick(1);
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
    arm7_memory.UpdateMemoryMap(0x03000000, 0x04000000);
    arm9_memory.UpdateMemoryMap(0x03000000, 0x04000000);
}

bool System::CartridgeAccessRights() {
    // check which cpu has access to the nds cartridge
    if (EXMEMCNT & (1 << 11)) {
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