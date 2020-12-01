#include <emulator/core/memory.h>
#include <stdio.h>

Memory::Memory(Emulator *emulator) : emulator(emulator) {

}

// u8 Memory::arm7_read_byte(u32 addr) {
//     switch (addr & 0xFF000000) {
//     case 0x00000000:
//         return arm7_bios[addr];
//     case 0x02000000:
//         return main_ram[addr - 0x02000000];
//     case 0x03000000:
//         if (addr >= 0x03800000) {
//             return arm7_wram[addr - 0x03800000];
//         } else {
//             return shared_wram[addr - 0x03000000];
//         }
//     // TODO: add wireless communications
//     case 0x04000000:
//         arm7_read_io(addr);
//     case 0x06000000:
//         return arm7_vram[addr - 0x06000000];
//     case 0x07000000:
//         return arm7_vram[addr - 0x07000000];
//     default:
//         printf("[Memory] byte read at address 0x%04x is unimplemented!\n", addr);
//     }
// }

// u16 Memory::arm7_read_halfword(u32 addr) {
//     return (arm7_read_byte(addr + 1) << 8 | arm7_read_byte(addr));
// }

// u32 Memory::arm7_read_word(u32 addr) {
//     return (arm7_read_halfword(addr + 2) << 16 | arm7_read_halfword(addr));
// }

// u32 Memory::arm7_read_io(u32 addr) {
//     switch (addr) {
//     default:
//         printf("[Memory] io read at address 0x%04x is unimplemented!\n", addr);
//     }
// }