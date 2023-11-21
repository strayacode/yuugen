#include "common/logger.h"
#include "common/regular_file.h"
#include "common/bits.h"
#include "common/memory.h"
#include "nds/arm7/arm7.h"
#include "nds/system.h"

namespace nds {

ARM7Memory::ARM7Memory(System& system) : system(system) {
    load_bios("../bios/bios7.bin");
}

void ARM7Memory::reset() {
    arm7_wram.fill(0);
    rcnt = 0;
    postflg = 0;
    biosprot = 0;

    map(0x00000000, 0x01000000, bios.data(), 0x3fff, arm::RegionAttributes::Read);
    map(0x02000000, 0x03000000, system.main_memory->data(), 0x3fffff, arm::RegionAttributes::ReadWrite);
    update_wram_mapping();
}

void ARM7Memory::update_wram_mapping() {
    switch (system.wramcnt) {
    case 0x0:
        map(0x03000000, 0x03800000, arm7_wram.data(), 0xffff, arm::RegionAttributes::ReadWrite);
        break;
    case 0x1:
        map(0x03000000, 0x03800000, system.shared_wram.data(), 0x3fff, arm::RegionAttributes::ReadWrite);
        break;
    case 0x2:
        map(0x03000000, 0x03800000, system.shared_wram.data() + 0x4000, 0x3fff, arm::RegionAttributes::ReadWrite);
        break;
    case 0x3:
        map(0x03000000, 0x03800000, system.shared_wram.data(), 0x7fff, arm::RegionAttributes::ReadWrite);
        break;
    }

    map(0x03800000, 0x04000000, arm7_wram.data(), 0xffff, arm::RegionAttributes::ReadWrite);
}

u8 ARM7Memory::read_byte(u32 addr) {
    switch (addr >> 24) {
    case 0x04:
        return mmio_read_byte(addr);
    case 0x06:
        return system.video_unit.vram.arm7_vram.read<u8>(addr);
    case 0x08:
    case 0x09:
        if (!common::get_bit<7>(system.exmemcnt)) {
            return 0;
        }
        
        return 0xff;
    default:
        logger.warn("ARM7Memory: handle 8-bit read %08x", addr);
    }

    return 0;
}

u16 ARM7Memory::read_half(u32 addr) {
    switch (addr >> 24) {
    case 0x04:
        return mmio_read_half(addr);
    case 0x06:
        return system.video_unit.vram.arm7_vram.read<u16>(addr);
    case 0x08:
    case 0x09:
        if (!common::get_bit<7>(system.exmemcnt)) {
            return 0;
        }
        
        return 0xffff;
    default:
        logger.warn("ARM7Memory: handle 16-bit read %08x", addr);
    }

    return 0;
}

u32 ARM7Memory::read_word(u32 addr) {
    switch (addr >> 24) {
    case 0x04:
        return mmio_read_word(addr);
    case 0x06:
        return system.video_unit.vram.arm7_vram.read<u32>(addr);
    case 0x08:
    case 0x09:
        if (!common::get_bit<7>(system.exmemcnt)) {
            return 0;
        }
        
        return 0xffffffff;
    default:
        logger.warn("ARM7Memory: handle 32-bit read %08x", addr);
    }

    return 0;
}

void ARM7Memory::write_byte(u32 addr, u8 value) {
    switch (addr >> 24) {
    case 0x04:
        mmio_write_byte(addr, value);
        break;
    case 0x06:
        system.video_unit.vram.arm7_vram.write<u8>(addr, value);
        break;
    default:
        logger.warn("ARM7Memory: handle 8-bit write %08x = %02x", addr, value);
    }
}

void ARM7Memory::write_half(u32 addr, u16 value) {
    switch (addr >> 24) {
    case 0x04:
        mmio_write_half(addr, value);
        break;
    case 0x06:
        system.video_unit.vram.arm7_vram.write<u16>(addr, value);
        break;
    default:
        logger.warn("ARM7Memory: handle 16-bit write %08x = %02x", addr, value);
    }
}

void ARM7Memory::write_word(u32 addr, u32 value) {
    switch (addr >> 24) {
    case 0x04:
        mmio_write_word(addr, value);
        break;
    case 0x06:
        system.video_unit.vram.arm7_vram.write<u32>(addr, value);
        break;
    case 0x08: case 0x09:
        // ignore gba cart writes
        break;
    default:
        logger.warn("ARM7Memory: handle 32-bit write %08x = %02x", addr, value);
    }
}

void ARM7Memory::load_bios(const std::string& path) {
    common::RegularFile file;
    file.load(path);
    bios = common::read<std::array<u8, 0x4000>>(file.get_pointer(0));
}

void ARM7Memory::write_postflg(u8 value) {
    postflg |= value & 0x1;
}

} // namespace nds