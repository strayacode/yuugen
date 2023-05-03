#include "common/logger.h"
#include "core/nds/arm7/arm7.h"
#include "core/nds/system.h"

namespace core::nds {

#define MMIO(addr) (addr >> 2)

ARM7Memory::ARM7Memory(System& system) : system(system) {}

void ARM7Memory::reset() {
    arm7_wram.fill(0);
    rcnt = 0;
    postflg = 0;

    map<arm::Bus::All>(0x02000000, 0x03000000, system.main_memory.data(), 0x3fffff, arm::RegionAttributes::ReadWrite);
    update_wram_mapping();
}

void ARM7Memory::update_wram_mapping() {
    switch (system.wramcnt) {
    case 0x0:
        map<arm::Bus::All>(0x03000000, 0x03800000, arm7_wram.data(), 0xffff, arm::RegionAttributes::ReadWrite);
        break;
    case 0x1:
        map<arm::Bus::All>(0x03000000, 0x03800000, system.shared_wram.data(), 0x3fff, arm::RegionAttributes::ReadWrite);
        break;
    case 0x2:
        map<arm::Bus::All>(0x03000000, 0x03800000, system.shared_wram.data() + 0x4000, 0x3fff, arm::RegionAttributes::ReadWrite);
        break;
    case 0x3:
        map<arm::Bus::All>(0x03000000, 0x03800000, system.shared_wram.data(), 0x7fff, arm::RegionAttributes::ReadWrite);
        break;
    }

    map<arm::Bus::All>(0x03800000, 0x04000000, arm7_wram.data(), 0xffff, arm::RegionAttributes::ReadWrite);
}

u8 ARM7Memory::read_byte(u32 addr) {
    switch (addr >> 24) {
    case 0x04:
        return mmio_read_byte(addr);
    default:
        logger.error("ARM7Memory: handle 8-bit read %08x", addr);
    }

    return 0;
}

u16 ARM7Memory::read_half(u32 addr) {
    switch (addr >> 24) {
    case 0x04:
        return mmio_read_half(addr);
    default:
        logger.error("ARM7Memory: handle 16-bit read %08x", addr);
    }

    return 0;
}

u32 ARM7Memory::read_word(u32 addr) {
    switch (addr >> 24) {
    case 0x04:
        return mmio_read_word(addr);
    default:
        logger.error("ARM7Memory: handle 32-bit read %08x", addr);
    }

    return 0;
}

void ARM7Memory::write_byte(u32 addr, u8 value) {
    switch (addr >> 24) {
    case 0x04:
        mmio_write_byte(addr, value);
        break;
    default:
        logger.error("ARM7Memory: handle 8-bit write %08x = %02x", addr, value);
    }
}

void ARM7Memory::write_half(u32 addr, u16 value) {
    switch (addr >> 24) {
    case 0x04:
        mmio_write_half(addr, value);
        break;
    default:
        logger.error("ARM7Memory: handle 16-bit write %08x = %02x", addr, value);
    }
}

void ARM7Memory::write_word(u32 addr, u32 value) {
    switch (addr >> 24) {
    case 0x04:
        mmio_write_word(addr, value);
        break;
    default:
        logger.error("ARM7Memory: handle 32-bit write %08x = %02x", addr, value);
    }
}

} // namespace core::nds