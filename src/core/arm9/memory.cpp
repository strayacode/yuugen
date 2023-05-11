#include "common/logger.h"
#include "core/arm9/arm9.h"
#include "core/system.h"

namespace core {

#define MMIO(addr) (addr >> 2)

ARM9Memory::ARM9Memory(System& system) : system(system) {}

void ARM9Memory::reset() {
    postflg = 0;

    map(0x02000000, 0x03000000, system.main_memory.data(), 0x3fffff, arm::RegionAttributes::ReadWrite);
    update_wram_mapping();
}

void ARM9Memory::update_wram_mapping() {
    switch (system.wramcnt) {
    case 0x0:
        map(0x03000000, 0x04000000, system.shared_wram.data(), 0x7fff, arm::RegionAttributes::ReadWrite);
        break;
    case 0x1:
        map(0x03000000, 0x04000000, system.shared_wram.data() + 0x4000, 0x3fff, arm::RegionAttributes::ReadWrite);
        break;
    case 0x2:
        map(0x03000000, 0x04000000, system.shared_wram.data(), 0x3fff, arm::RegionAttributes::ReadWrite);
        break;
    case 0x3:
        unmap(0x03000000, 0x04000000, arm::RegionAttributes::ReadWrite);
        break;
    }
}

u8 ARM9Memory::read_byte(u32 addr) {
    switch (addr >> 24) {
    case 0x04:
        return mmio_read_byte(addr);
    default:
        logger.error("ARM9Memory: handle 8-bit read %08x", addr);
    }

    return 0;
}

u16 ARM9Memory::read_half(u32 addr) {
    switch (addr >> 24) {
    case 0x04:
        return mmio_read_half(addr);
    default:
        logger.error("ARM9Memory: handle 16-bit read %08x", addr);
    }

    return 0;
}

u32 ARM9Memory::read_word(u32 addr) {
    switch (addr >> 24) {
    case 0x04:
        return mmio_read_word(addr);
    default:
        logger.error("ARM9Memory: handle 32-bit read %08x", addr);
    }

    return 0;
}

void ARM9Memory::write_byte(u32 addr, u8 value) {
    switch (addr >> 24) {
    case 0x04:
        mmio_write_byte(addr, value);
        break;
    default:
        logger.error("ARM9Memory: handle 8-bit write %08x = %02x", addr, value);
    }
}

void ARM9Memory::write_half(u32 addr, u16 value) {
    switch (addr >> 24) {
    case 0x04:
        mmio_write_half(addr, value);
        break;
    case 0x06:
        system.video_unit.vram.write<u16>(addr, value);
        break;
    default:
        logger.error("ARM9Memory: handle 16-bit write %08x = %02x", addr, value);
    }
}

void ARM9Memory::write_word(u32 addr, u32 value) {
    switch (addr >> 24) {
    case 0x04:
        mmio_write_word(addr, value);
        break;
    default:
        logger.error("ARM9Memory: handle 32-bit write %08x = %02x", addr, value);
    }
}

} // namespace core