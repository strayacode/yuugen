#include "common/logger.h"
#include "common/regular_file.h"
#include "common/bits.h"
#include "common/memory.h"
#include "gba/system.h"

namespace gba {

Memory::Memory(System& system) : system(system) {
    load_bios("../bios/gba_bios.bin");
}

void Memory::reset() {
    ewram.fill(0);
    iwram.fill(0);

    map(0x00000000, 0x01000000, bios.data(), 0x3fff, arm::RegionAttributes::Read);
    map(0x02000000, 0x02040000, ewram.data(), 0x3ffff, arm::RegionAttributes::ReadWrite);
    map(0x03000000, 0x03008000, iwram.data(), 0x7fff, arm::RegionAttributes::ReadWrite);

    // TODO: how does the rom get mirrored if it's < 32mb?
    map(0x08000000, 0x0a000000, system.cartridge.get_rom_pointer(), 0x1ffffff, arm::RegionAttributes::Read);
}

u8 Memory::read_byte(u32 addr) {
    switch (addr >> 24) {
    default:
        logger.todo("Memory: handle 8-bit read %08x", addr);
    }

    return 0;
}

u16 Memory::read_half(u32 addr) {
    switch (addr >> 24) {
    default:
        logger.todo("Memory: handle 16-bit read %08x", addr);
    }

    return 0;
}

u32 Memory::read_word(u32 addr) {
    switch (addr >> 24) {
    default:
        logger.todo("Memory: handle 32-bit read %08x", addr);
    }

    return 0;
}

void Memory::write_byte(u32 addr, u8 value) {
    switch (addr >> 24) {
    case 0x04:
        mmio_write_byte(addr, value);
        break;
    default:
        logger.todo("Memory: handle 8-bit write %08x = %02x", addr, value);
    }
}

void Memory::write_half(u32 addr, u16 value) {
    switch (addr >> 24) {
    case 0x04:
        mmio_write_half(addr, value);
        break;
    default:
        logger.todo("Memory: handle 16-bit write %08x = %02x", addr, value);
    }
}

void Memory::write_word(u32 addr, u32 value) {
    switch (addr >> 24) {
    case 0x04:
        mmio_write_word(addr, value);
        break;
    default:
        logger.todo("Memory: handle 32-bit write %08x = %02x", addr, value);
    }
}

void Memory::load_bios(const std::string& path) {
    common::RegularFile file;
    file.load(path);
    bios = common::read<std::array<u8, 0x4000>>(file.get_pointer(0));
}

} // namespace gba