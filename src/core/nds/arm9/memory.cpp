#include "common/logger.h"
#include "core/nds/arm9/arm9.h"
#include "core/nds/system.h"

namespace core::nds {

ARM9Memory::ARM9Memory(System& system) : system(system) {}

void ARM9Memory::reset() {
    
}

void ARM9Memory::update_memory_map() {
    using Bus = arm::Bus;
    map<Bus::Code>()
}

u8 ARM9Memory::system_read_byte(u32 addr) {
    switch (addr) {
    default:
        logger.debug("ARM9Memory: unmapped byte read at %08x", addr);
        break;
    }
    
    return 0;
}

u16 ARM9Memory::system_read_half(u32 addr) {
    switch (addr) {
    default:
        logger.debug("ARM9Memory: unmapped half read at %08x", addr);
        break;
    }

    return 0;
}

u32 ARM9Memory::system_read_word(u32 addr) {
    switch (addr) {
    default:
        logger.debug("ARM9Memory: unmapped word read at %08x", addr);
        break;
    }

    return 0;
}

void ARM9Memory::system_write_byte(u32 addr, u8 value) {
    switch (addr) {
    default:
        logger.debug("ARM9Memory: unmapped byte read at %08x = %02x", addr, value);
        break;
    }
}

void ARM9Memory::system_write_half(u32 addr, u16 value) {
    switch (addr) {
    default:
        logger.debug("ARM9Memory: unmapped half write at %08x = %04x", addr, value);
        break;
    }
}

void ARM9Memory::system_write_word(u32 addr, u32 value) {
    switch (addr) {
    default:
        logger.debug("ARM9Memory: unmapped word write at %08x = %08x", addr, value);
        break;
    }
}

} // namespace core::nds