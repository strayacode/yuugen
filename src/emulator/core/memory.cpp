#include <emulator/core/memory.h>
#include <emulator/emulator.h>
#include <emulator/core/gpu.h>
#include <stdio.h>
#include <emulator/common/log.h>
#include <string>
#include <string.h>
#include <emulator/common/arithmetic.h>

static const int ARM9_BIOS_SIZE = 32 * 1024;
static const int ARM7_BIOS_SIZE = 16 * 1024;
static const int FIRMWARE_SIZE = 256 * 1024;

Memory::Memory(Emulator *emulator) : emulator(emulator) {

}

template <typename T>
T Memory::arm7_read(u32 addr) {
    u8 *data_addr = nullptr;
    switch (addr & 0xFF000000) {
    // main memory
    case 0x02000000:
        data_addr = &main_ram[addr & 0x3FFFFF];
        break;
    // shared wram
    case 0x03000000:
        // arm7 wram
        if (addr >= 0x03800000) {
            data_addr = &arm7_wram[addr & 0xFFFF];
            
        }
        break;
    default:
        log_fatal("[Memory] read from arm7 at address 0x%04x is unimplemented!\n", addr);
    }

    T return_value = 0;
    for (u32 i = 0; i < sizeof(T); i++) {
        return_value |= (data_addr[i] << (i * 8));
    }
    return return_value;
}

template <typename T>
T Memory::arm9_read(u32 addr) {
    u8 *data_addr = nullptr;
    switch (addr & 0xFF000000) {
    // case 0x00000000:
    //     // for now just read from instruction tcm whatever
    case 0x02000000:
        data_addr = &main_ram[addr & 0x3FFFFF];
        main_ram[addr & 0x3FFFFF + 3] << 24 | 
        break;
        // case 0x03000000:
        //     return shared_wram[addr - 0x03000000];
        // case 0x04000000:
        //     return arm9_read_io(addr);
        // case 0x05000000:
        //     return palette_ram[addr - 0x05000000];
        // case 0x06000000:
        //     // deal with later lol
        // case 0x07000000:
        //     return oam[addr - 0x07000000];
    
    case 0xFF000000:
        data_addr = &arm9_bios[addr & 0x7FFF];
        break;
    default:
        log_fatal("[Memory] read from arm9 at address 0x%04x is unimplemented!\n", addr);
    }
    
    T return_value = 0;
    for (u32 i = 0; i < sizeof(T); i++) {
        return_value |= (data_addr[i] << (i * 8));
    }

    return return_value;

}

template <typename T>
T Memory::arm7_read_io(u32 addr) {
    switch (addr) {
    default:
        log_fatal("[Memory] io read by arm7 at address 0x%04x is unimplemented!\n", addr);
    }
}

template <typename T>
T Memory::arm9_read_io(u32 addr) {
    switch (addr) {
    default:
        log_fatal("[Memory] io read by arm9 at address 0x%04x is unimplemented!\n", addr);
    }
}

template <typename T>
void Memory::arm7_write(u32 addr, T data) {
    u8 *data_addr = nullptr;
    switch (addr & 0xFF000000) {
    case 0x02000000:
        data_addr = &main_ram[addr & 0x3FFFFF];
        break;
    case 0x03000000:
        if (addr >= 0x03800000) {
            data_addr = &arm7_wram[addr & 0xFFFF];
        }
        break;
    default:
        log_fatal("[Memory] write from arm7 to address 0x%04x is unimplemented!\n", addr);
    }
    for (u32 i = 0; i < sizeof(T); i++) {
        data_addr[i] = (data >> (i * 8));
    }

}

template <typename T>
void Memory::arm9_write(u32 addr, T data) {
    // printf("%04x\n", addr);
    u8 *data_addr = nullptr;
    switch (addr & 0xFF000000) {
    case 0x02000000:
        data_addr = &main_ram[addr & 0x3FFFFF];
        break;
    case 0x03000000:
        printf("shared wram\n");
        break;
    case 0x04000000:
        arm9_write_io<T>(addr, data);
        return; // since write is already done
    case 0x06000000:
        // ignore 8 bit writes
        if (sizeof(T) == 1) {
            break;
        }
        if (addr >= 0x06800000) {
            // undefined reference
            emulator->gpu.write_lcdc<T>(addr, data);
            return; // since write is already done
        }
        break;
    default:
        log_fatal("[Memory] write from arm9 at address 0x%04x is unimplemented!\n", addr);
    }
    for (u32 i = 0; i < sizeof(T); i++) {
        data_addr[i] = (data >> (i * 8));
    }
}

template <typename T>
void Memory::arm7_write_io(u32 addr, T data) {
    switch (addr) {
    default:
        log_fatal("[Memory] io write by arm7 at address 0x%04x is unimplemented!\n", addr);
    }
}

template <typename T> 
void Memory::arm9_write_io(u32 addr, T data) {
    switch (addr) {
    case 0x04000000:
        emulator->gpu.engine_a.dispcnt = data;
        break;
    case 0x040000D0:
        emulator->dma->chan[2].word_count = data;
        break;
    case 0x04000240:
        emulator->gpu.vramcnt_a = data;
        break;
    case 0x04000241:
        emulator->gpu.vramcnt_b = data;
        break;
    case 0x04000304:
        emulator->gpu.powcnt1 = data;
        break;
    default:
        log_fatal("[Memory] io write by arm9 at address 0x%04x with data 0x%04x is unimplemented!", addr, data);
    }
}

void Memory::load_arm9_bios() {
    FILE *file_buffer = fopen("../bios/bios9.bin", "rb");
    if (file_buffer == NULL) {
        log_fatal("[Memory] error when opening arm9 bios! make sure the file bios9.bin exists in the bios folder");
    }
    fseek(file_buffer, 0, SEEK_END);
    fseek(file_buffer, 0, SEEK_SET);
    fread(arm9_bios, ARM9_BIOS_SIZE, 1, file_buffer);
    fclose(file_buffer);  
    log_debug("[Memory] arm9 bios loaded successfully!");
}

void Memory::load_arm7_bios() {
    FILE *file_buffer = fopen("../bios/bios7.bin", "rb");
    if (file_buffer == NULL) {
        log_fatal("[Memory] error when opening arm7 bios! make sure the file bios7.bin exists in the bios folder\n");
    }
    fseek(file_buffer, 0, SEEK_END);
    fseek(file_buffer, 0, SEEK_SET);
    fread(arm7_bios, ARM7_BIOS_SIZE, 1, file_buffer);
    fclose(file_buffer);  
    log_debug("[Memory] arm7 bios loaded successfully!");
}

void Memory::load_firmware() {
    FILE *file_buffer = fopen("../firmware/firmware.bin", "rb");
    if (file_buffer == NULL) {
        log_fatal("[Memory] error when opening firmware! make sure the file firmware.bin exists in the firmware folder\n");
    }
    fseek(file_buffer, 0, SEEK_END);
    fseek(file_buffer, 0, SEEK_SET);
    fread(firmware, FIRMWARE_SIZE, 1, file_buffer);
    fclose(file_buffer);  
    log_debug("[Memory] firmware loaded successfully!");
}

u8 Memory::arm7_read_byte(u32 addr) {
    return arm7_read<u8>(addr);
}

u16 Memory::arm7_read_halfword(u32 addr) {
    return arm7_read<u16>(addr);
}

u32 Memory::arm7_read_word(u32 addr) {
    return arm7_read<u32>(addr);
}

u8 Memory::arm9_read_byte(u32 addr) {
    return arm9_read<u8>(addr);
}

u16 Memory::arm9_read_halfword(u32 addr) {
    return arm9_read<u16>(addr);
}

u32 Memory::arm9_read_word(u32 addr) {
    return arm9_read<u32>(addr);
}

void Memory::arm7_write_byte(u32 addr, u8 data) {
    arm7_write<u8>(addr, data);
}

void Memory::arm7_write_halfword(u32 addr, u16 data) {
    
    arm7_write<u16>(addr, data);
}

void Memory::arm7_write_word(u32 addr, u32 data) {
    arm7_write<u32>(addr, data);
}

void Memory::arm9_write_byte(u32 addr, u8 data) {
    arm9_write<u8>(addr, data);
}

void Memory::arm9_write_halfword(u32 addr, u16 data) { 
    arm9_write<u16>(addr, data);
}

void Memory::arm9_write_word(u32 addr, u32 data) {
    arm9_write<u32>(addr, data);
}

