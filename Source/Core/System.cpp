#include <algorithm>
#include "Core/System.h"

System::System() 
    :video_unit(*this), cartridge(*this),
    spi(*this),
    dma {DMA(*this, 0), DMA(*this, 1)}, ipc(*this), 
    timers {Timers(*this, 0), Timers(*this, 1)},
    spu(*this),
    arm7(*this), arm9(*this) {
    arm7.select_backend(CPUBackend::Interpreter);
    arm9.select_backend(CPUBackend::Interpreter);

    arm7.memory().build_mmio();
    arm9.memory().build_mmio();
}

void System::Reset() {
    arm7.memory().reset();
    arm9.memory().reset();
    scheduler.reset();
    cartridge.reset();
    cartridge.LoadRom(rom_path);
    video_unit.reset();
    dma[0].reset();
    dma[1].reset();
    timers[0].reset();
    timers[1].reset();
    arm9.coprocessor().reset();
    spi.reset();
    input.reset();
    ipc.reset();
    maths_unit.reset();
    spu.reset();
    rtc.reset();

    main_memory.fill(0);
    shared_wram.fill(0);
    
    wramcnt = 0;
    powcnt2 = 0;
    rcnt = 0;
    HALTCNT = 0;
    exmemcnt = 0;
    postflg7 = 0;
    postflg9 = 0;
    biosprot = 0;
    siocnt = 0;

    arm7.cpu().reset();
    arm9.cpu().reset();
}

void System::DirectBoot() {
    arm9.coprocessor().direct_boot();

    // TODO: make a direct_boot function for ARM7Memory and ARM9Memory
    arm7.memory().write<u16>(0x04000134, 0x8000); // rcnt
    arm9.memory().write<u8>(0x04000247, 0x03); // wramcnt
    arm9.memory().write<u8>(0x04000300, 0x01); // POSTFLG (ARM9)
    arm7.memory().write<u8>(0x04000300, 0x01); // POSTFLG (ARM7)
    arm9.memory().write<u16>(0x04000304, 0x0001); // POWCNT1
    arm7.memory().write<u16>(0x04000504, 0x0200); // SOUNDBIAS
    arm9.memory().write<u32>(0x027FF800, 0x00001FC2); // Chip ID 1
    arm9.memory().write<u32>(0x027FF804, 0x00001FC2); // Chip ID 2
    arm9.memory().write<u16>(0x027FF850, 0x5835); // ARM7 BIOS CRC
    arm9.memory().write<u16>(0x027FF880, 0x0007); // Message from ARM9 to ARM7
    arm9.memory().write<u16>(0x027FF884, 0x0006); // ARM7 boot task
    arm9.memory().write<u32>(0x027FFC00, 0x00001FC2); // Copy of chip ID 1
    arm9.memory().write<u32>(0x027FFC04, 0x00001FC2); // Copy of chip ID 2
    arm9.memory().write<u16>(0x027FFC10, 0x5835); // Copy of ARM7 BIOS CRC
    arm9.memory().write<u16>(0x027FFC40, 0x0001); // Boot indicator

    cartridge.DirectBoot();
    arm7.cpu().direct_boot(cartridge.loader.GetARM7Entrypoint());
    arm9.cpu().direct_boot(cartridge.loader.GetARM9Entrypoint());    
    spi.DirectBoot();
}

void System::FirmwareBoot() {
    arm7.cpu().firmware_boot();
    arm9.cpu().firmware_boot();
    cartridge.FirmwareBoot();
}

void System::SetGamePath(std::string path) {
    rom_path = path;
}

void System::RunFrame() {
    while (running) {
        u64 frame_end_time = scheduler.GetCurrentTime() + 560190;

        while (scheduler.GetCurrentTime() < frame_end_time) {
            if (!arm7.cpu().is_halted() || !arm9.cpu().is_halted()) {
                u64 cycles = std::min(static_cast<u64>(16), scheduler.GetEventTime() - scheduler.GetCurrentTime());
                u64 arm7_target = scheduler.GetCurrentTime() + cycles;
                u64 arm9_target = scheduler.GetCurrentTime() + (cycles << 1);
                
                // run the arm9 until the next scheduled event
                if (!arm9.cpu().run(arm9_target)) {
                    running = false;
                    return;
                }

                // let the arm7 catch up
                if(!arm7.cpu().run(arm7_target)) {
                    running = false;
                    return;
                }

                // advance the scheduler
                scheduler.Tick(cycles);
            } else {
                // if both cpus are halted we can just advance to the next event
                scheduler.set_current_time(scheduler.GetEventTime());
            }

            scheduler.RunEvents();
        }
    }
}

void System::write_haltcnt(u8 data) {
    HALTCNT = data & 0xC0;

    u8 power_down_mode = (HALTCNT >> 6) & 0x3;

    // check bits 6..7 to see what to do
    switch (power_down_mode) {
    case 2:
        arm7.cpu().halt();
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
    arm7.memory().update_memory_map(0x03000000, 0x04000000);
    arm9.memory().update_memory_map(0x03000000, 0x04000000);
}

bool System::CartridgeAccessRights() {
    // check which cpu has access to the nds cartridge
    if (exmemcnt & (1 << 11)) {
        return false; // 0 = ARMv4
    } else {
        return true; // 1 = ARMv5
    }
}
