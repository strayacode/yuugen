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
    map(0x00000000, 0x01000000, bios.data(), 0x3fff, arm::RegionAttributes::Read);
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
    default:
        logger.todo("Memory: handle 8-bit write %08x = %02x", addr, value);
    }
}

void Memory::write_half(u32 addr, u16 value) {
    switch (addr >> 24) {
    default:
        logger.todo("Memory: handle 16-bit write %08x = %02x", addr, value);
    }
}

void Memory::write_word(u32 addr, u32 value) {
    switch (addr >> 24) {
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