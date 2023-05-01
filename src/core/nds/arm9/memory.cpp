#include "common/logger.h"
#include "core/nds/arm9/arm9.h"
#include "core/nds/system.h"

namespace core::nds {

#define MMIO(addr) (addr >> 2)

ARM9Memory::ARM9Memory(System& system) : system(system) {}

void ARM9Memory::reset() {
    using Bus = arm::Bus;
    using RegionAttributes = arm::RegionAttributes;
    map<Bus::All>(0x02000000, 0x03000000, system.main_memory.data(), 0x3fffff, RegionAttributes::ReadWrite);
}

void ARM9Memory::update_memory_map() {
    
}

u8 ARM9Memory::read_byte(u32 addr) {
    switch (addr & 0x3) {
    case 0x0:
        return read_word<0x000000ff>(addr & ~0x3);
    case 0x1:
        return read_word<0x0000ff00>(addr & ~0x3) >> 8;
    case 0x2:
        return read_word<0x00ff0000>(addr & ~0x3) >> 16;
    case 0x3:
        return read_word<0xff000000>(addr & ~0x3) >> 24;
    }

    return 0;
}

u16 ARM9Memory::read_half(u32 addr) {
    switch (addr & 0x2) {
    case 0x0:
        return read_word<0x0000ffff>(addr & ~0x2);
    case 0x2:
        return read_word<0xffff0000>(addr & ~0x2) >> 16;
    }

    return 0;
}

u32 ARM9Memory::read_word(u32 addr) {
    return read_word<0xffffffff>(addr);
}

void ARM9Memory::write_byte(u32 addr, u8 value) {
    u32 mirrored = value * 0x01010101;
    switch (addr & 0x3) {
    case 0x0:
        write_word<0x000000ff>(addr & ~0x3, mirrored);
        break;
    case 0x1:
        write_word<0x0000ff00>(addr & ~0x3, mirrored);
        break;
    case 0x2:
        write_word<0x00ff0000>(addr & ~0x3, mirrored);
        break;
    case 0x3:
        write_word<0xff000000>(addr & ~0x3, mirrored);
        break;
    }
}

void ARM9Memory::write_half(u32 addr, u16 value) {
    u32 mirrored = value * 0x00010001;
    switch (addr & 0x2) {
    case 0x0:
        write_word<0x0000ffff>(addr & ~0x2, mirrored);
        break;
    case 0x2:
        write_word<0xffff0000>(addr & ~0x2, mirrored);
        break;
    }
}

void ARM9Memory::write_word(u32 addr, u32 value) {
    write_word<0xffffffff>(addr, value);
}

template <u32 mask>
u32 ARM9Memory::read_word(u32 addr) {
    switch (MMIO(addr)) {
    default:
        logger.error("ARM9Memory: unmapped read mask=%08x", mask);
        break;
    }

    return 0;
}

template <u32 mask>
void ARM9Memory::write_word(u32 addr, u32 value) {
    switch (MMIO(addr)) {
    default:
        logger.error("ARM9Memory: unmapped write mask=%08x", mask);
        break;
    }
}

} // namespace core::nds