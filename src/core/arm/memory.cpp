#include <core/arm/memory.h>
#include <core/core.h>

Memory::Memory(Core* core) : core(core) {
    
}

void Memory::Reset() {
    memset(main_memory, 0, 0x400000);
    memset(arm7_wram, 0, 0x10000);
    memset(shared_wram, 0, 0x8000);
    memset(arm7_bios, 0, 0x4000);
    memset(arm9_bios, 0, 0x8000);

    WRAMCNT = 0;
    POWCNT2 = 0;
    RCNT = 0;
    HALTCNT = 0;
    EXMEMCNT = 0;
    POSTFLG7 = 0;
    POSTFLG9 = 0;

    LoadARM7Bios();
    LoadARM9Bios();
}

void Memory::DirectBoot() {
    WRAMCNT = 3;
    RCNT = 0x8000;
    POSTFLG7 = 1;
    POSTFLG9 = 1;
}

void Memory::LoadARM7Bios() {
    FILE* file_buffer = fopen("../bios/bios7.bin", "rb");
    if (file_buffer == NULL) {
        log_fatal("error when opening the arm7 bios! make sure the file bios7.bin exists in the bios folder");
    }
    fseek(file_buffer, 0, SEEK_END);
    fseek(file_buffer, 0, SEEK_SET);
    fread(arm7_bios, 0x4000, 1, file_buffer);
    fclose(file_buffer);  
    log_debug("[ARM7] Bios loaded successfully");
}

void Memory::LoadARM9Bios() {
    FILE* file_buffer = fopen("../bios/bios9.bin", "rb");
    if (file_buffer == NULL) {
        log_fatal("error when opening the arm9 bios! make sure the file bios9.bin exists in the bios folder");
    }
    fseek(file_buffer, 0, SEEK_END);
    fseek(file_buffer, 0, SEEK_SET);
    fread(arm9_bios, 0x8000, 1, file_buffer);
    fclose(file_buffer);  
    log_debug("[ARM9] Bios loaded successfully!");
}

void Memory::WriteHALTCNT(u8 data) {
    HALTCNT = data & 0xC0;

    u8 power_down_mode = (HALTCNT >> 6) & 0x3;

    // check bits 6..7 to see what to do
    switch (power_down_mode) {
    case 2:
        core->arm7.Halt();
        break;
    default:
        log_fatal("power down mode %d is not implemented!", power_down_mode);
        break;
    }
}