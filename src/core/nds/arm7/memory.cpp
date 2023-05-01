#include "common/logger.h"
#include "core/nds/arm7/arm7.h"
#include "core/nds/system.h"

namespace core::nds {

#define MMIO(addr) (addr >> 2)

ARM7Memory::ARM7Memory(System& system) : system(system) {}

void ARM7Memory::reset() {
    arm7_wram.fill(0);
    rcnt = 0;

    map<arm::Bus::All>(0x02000000, 0x03000000, system.main_memory.data(), 0x3fffff, arm::RegionAttributes::ReadWrite);
    map_wram_region();
}

void ARM7Memory::map_wram_region() {
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

u16 ARM7Memory::read_half(u32 addr) {
    switch (addr & 0x2) {
    case 0x0:
        return read_word<0x0000ffff>(addr & ~0x2);
    case 0x2:
        return read_word<0xffff0000>(addr & ~0x2) >> 16;
    }

    return 0;
}

u32 ARM7Memory::read_word(u32 addr) {
    return read_word<0xffffffff>(addr);
}

void ARM7Memory::write_byte(u32 addr, u8 value) {
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

void ARM7Memory::write_half(u32 addr, u16 value) {
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

void ARM7Memory::write_word(u32 addr, u32 value) {
    write_word<0xffffffff>(addr, value);
}

template <u32 mask>
u32 ARM7Memory::read_word(u32 addr) {
    switch (MMIO(addr)) {
    default:
        logger.error("ARM7Memory: unmapped read mask=%08x", mask);
        break;
    }

    return 0;
}

template <u32 mask>
void ARM7Memory::write_word(u32 addr, u32 value) {
    switch (MMIO(addr)) {
    case MMIO(0x04000134): 
        if constexpr (mask & 0xffff) {
            rcnt = value;
        }
    default:
        logger.error("ARM7Memory: unmapped %d-bit write %08x = %08x", get_access_size(mask), addr + get_access_offset(mask), (value & mask) >> (get_access_offset(mask) * 8));
        break;
    }
}

int ARM7Memory::get_access_size(u32 mask) {
    int size = 0;
    for (int i = 0; i < 4; i++) {
        if (mask & 0xff) {
            size += 8;
        }

        mask >>= 8;
    }

    return size;
}

u32 ARM7Memory::get_access_offset(u32 mask) {
    u32 offset = 0;
    for (int i = 0; i < 4; i++) {
        if (mask & 0xff) {
            break;
        }

        offset++;
        mask >>= 8;
    }

    return offset;
}

} // namespace core::nds