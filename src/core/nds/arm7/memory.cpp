#include "common/logger.h"
#include "core/nds/arm7/arm7.h"
#include "core/nds/system.h"

namespace core::nds {

ARM7Memory::ARM7Memory(System& system) : system(system) {}

void ARM7Memory::reset() {
    arm7_wram.fill(0);
    map<arm::Bus::All>(0x02000000, 0x03000000, system.main_memory.data(), 0x3fffff, arm::RegionAttributes::ReadWrite);
    map<arm::Bus::All>(0x03800000, 0x04000000, arm7_wram.data(), 0xffff, arm::RegionAttributes::ReadWrite);
}

void ARM7Memory::update_memory_map() {

}

u8 ARM7Memory::system_read_byte(u32 addr) {
    switch (addr) {
    default:
        logger.debug("ARM7Memory: unmapped byte read at %08x", addr);
        break;
    }
    
    return 0;
}

u16 ARM7Memory::system_read_half(u32 addr) {
    switch (addr) {
    default:
        logger.debug("ARM7Memory: unmapped half read at %08x", addr);
        break;
    }

    return 0;
}

u32 ARM7Memory::system_read_word(u32 addr) {
    switch (addr) {
    default:
        logger.debug("ARM7Memory: unmapped word read at %08x", addr);
        break;
    }

    return 0;
}

void ARM7Memory::system_write_byte(u32 addr, u8 value) {
    switch (addr) {
    default:
        logger.debug("ARM7Memory: unmapped byte read at %08x = %02x", addr, value);
        break;
    }
}

void ARM7Memory::system_write_half(u32 addr, u16 value) {
    switch (addr) {
    default:
        logger.debug("ARM7Memory: unmapped half write at %08x = %04x", addr, value);
        break;
    }
}

void ARM7Memory::system_write_word(u32 addr, u32 value) {
    switch (addr) {
    default:
        logger.debug("ARM7Memory: unmapped word write at %08x = %08x", addr, value);
        break;
    }
}

} // namespace core::nds