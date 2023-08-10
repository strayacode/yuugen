#include "common/logger.h"
#include "common/regular_file.h"
#include "common/bits.h"
#include "core/arm9/arm9.h"
#include "core/system.h"

namespace core {

ARM9Memory::ARM9Memory(System& system) : system(system) {
    load_bios("../bios/bios9.bin");
}

void ARM9Memory::reset() {
    postflg = 0;
    dtcm_data.fill(0);
    itcm_data.fill(0);

    dtcm.data = dtcm_data.data();
    dtcm.mask = dtcm_data.size() - 1;
    itcm.data = itcm_data.data();
    itcm.mask = itcm_data.size() - 1;

    map(0xffff0000, 0xffff8000, bios.data(), 0x7fff, arm::RegionAttributes::Read);
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
    case 0x06:
        return system.video_unit.vram.read<u8>(addr);
    case 0x08:
    case 0x09:
        if (common::get_bit<7>(system.exmemcnt)) {
            return 0;
        }
        
        return 0xff;
    default:
        logger.warn("ARM9Memory: handle 8-bit read %08x", addr);
    }

    return 0;
}

u16 ARM9Memory::read_half(u32 addr) {
    switch (addr >> 24) {
    case 0x04:
        return mmio_read_half(addr);
    case 0x05:
        return system.video_unit.read_palette_ram<u16>(addr);
    case 0x07:
        return system.video_unit.read_oam<u16>(addr);
    case 0x06:
        return system.video_unit.vram.read<u16>(addr);
    case 0x08:
    case 0x09:
        if (common::get_bit<7>(system.exmemcnt)) {
            return 0;
        }
        
        return 0xffff;
    default:
        logger.warn("ARM9Memory: handle 16-bit read %08x", addr);
    }

    return 0;
}

u32 ARM9Memory::read_word(u32 addr) {
    switch (addr >> 24) {
    case 0x04:
        return mmio_read_word(addr);
    case 0x06:
        return system.video_unit.vram.read<u32>(addr);
    case 0x08:
    case 0x09:
        if (common::get_bit<7>(system.exmemcnt)) {
            return 0;
        }

        return 0xffffffff;
    case 0x0a:
        return 0;
    default:
        logger.warn("ARM9Memory: handle 32-bit read %08x", addr);
    }

    return 0;
}

void ARM9Memory::write_byte(u32 addr, u8 value) {
    switch (addr >> 24) {
    case 0x04:
        mmio_write_byte(addr, value);
        break;
    case 0x06:
        system.video_unit.vram.write<u8>(addr, value);
        break;
    default:
        logger.warn("ARM9Memory: handle 8-bit write %08x = %02x", addr, value);
    }
}

void ARM9Memory::write_half(u32 addr, u16 value) {
    switch (addr >> 24) {
    case 0x04:
        mmio_write_half(addr, value);
        break;
    case 0x05:
        system.video_unit.write_palette_ram<u16>(addr, value);
        break;
    case 0x06:
        system.video_unit.vram.write<u16>(addr, value);
        break;
    case 0x07:
        system.video_unit.write_oam<u16>(addr, value);
        break;
    default:
        logger.warn("ARM9Memory: handle 16-bit write %08x = %02x", addr, value);
    }
}

void ARM9Memory::write_word(u32 addr, u32 value) {
    switch (addr >> 24) {
    case 0x04:
        mmio_write_word(addr, value);
        break;
    case 0x05:
        system.video_unit.write_palette_ram<u32>(addr, value);
        break;
    case 0x06:
        system.video_unit.vram.write<u32>(addr, value);
        break;
    case 0x07:
        system.video_unit.write_oam<u32>(addr, value);
        break;
    case 0x08: case 0x09:
        // ignore gba cart writes
        break;
    default:
        logger.warn("ARM9Memory: handle 32-bit write %08x = %02x", addr, value);
    }
}

void ARM9Memory::load_bios(const std::string& path) {
    common::RegularFile file;
    file.load(path);
    bios = common::read<std::array<u8, 0x8000>>(file.get_pointer(0));
}

} // namespace core