#include <core/arm/memory.h>
#include <core/core.h>

Memory::Memory(Core* core) : core(core) {

}

void Memory::Reset() {
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

    LoadARM7Bios();
    LoadARM9Bios();

    // setup the arm7 and arm9 page tables
    UpdateARM7MemoryMap(0, 0xFFFFFFFF);
    UpdateARM9MemoryMap(0, 0xFFFFFFFF);
}

void Memory::DirectBoot() {
    RCNT = 0x8000;

    ARM9Write<u8>(0x4000247, 0x03); // WRAMCNT
    ARM9Write<u8>(0x4000300, 0x01); // POSTFLG (ARM9)
    ARM7Write<u8>(0x4000300, 0x01); // POSTFLG (ARM7)
    ARM9Write<u16>(0x4000304, 0x0001); // POWCNT1
    ARM7Write<u16>(0x4000504, 0x0200); // SOUNDBIAS

    // Set some memory values as the BIOS/firmware would
    ARM9Write<u32>(0x27FF800, 0x00001FC2); // Chip ID 1
    ARM9Write<u32>(0x27FF804, 0x00001FC2); // Chip ID 2
    ARM9Write<u16>(0x27FF850, 0x5835); // ARM7 BIOS CRC
    ARM9Write<u16>(0x27FF880, 0x0007); // Message from ARM9 to ARM7
    ARM9Write<u16>(0x27FF884, 0x0006); // ARM7 boot task
    ARM9Write<u32>(0x27FFC00, 0x00001FC2); // Copy of chip ID 1
    ARM9Write<u32>(0x27FFC04, 0x00001FC2); // Copy of chip ID 2
    ARM9Write<u16>(0x27FFC10, 0x5835); // Copy of ARM7 BIOS CRC
    ARM9Write<u16>(0x27FFC40, 0x0001); // Boot indicator
}

void Memory::LoadARM7Bios() {
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

void Memory::LoadARM9Bios() {
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

void Memory::WriteHALTCNT(u8 data) {
    HALTCNT = data & 0xC0;

    u8 power_down_mode = (HALTCNT >> 6) & 0x3;

    // check bits 6..7 to see what to do
    switch (power_down_mode) {
    case 2:
        core->arm7.Halt();
        break;
    case 3:
        log_warn("unhandled request for sleep mode");
        break;
    default:
        log_fatal("power down mode %d is not implemented!", power_down_mode);
        break;
    }
}

auto Memory::CartridgeAccessRights() -> bool {
    // check which cpu has access to the nds cartridge
    if (EXMEMCNT & (1 << 11)) {
        return false; // 0 = ARMv4
    } else {
        return true; // 1 = ARMv5
    }
}

void Memory::WriteWRAMCNT(u8 data) {
    WRAMCNT = data & 0x3;

    // now we must update the memory map for the shared wram space specifically
    UpdateARM7MemoryMap(0x03000000, 0x04000000);
    UpdateARM9MemoryMap(0x03000000, 0x04000000);
}