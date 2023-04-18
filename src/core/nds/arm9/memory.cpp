#include "core/nds/arm9/arm9.h"
#include "core/nds/system.h"

namespace core::nds {

ARM9Memory::ARM9Memory(System& system) : system(system) {}

u8 ARM9Memory::system_read_byte(u32 addr) {
    return 0;
}

u16 ARM9Memory::system_read_half(u32 addr) {
    return 0;
}

u32 ARM9Memory::system_read_word(u32 addr) {
    return 0;
}

void ARM9Memory::system_write_byte(u32 addr, u8 value) {

}

void ARM9Memory::system_write_half(u32 addr, u16 value) {

}

void ARM9Memory::system_write_word(u32 addr, u32 value) {

}

void ARM9Memory::direct_boot() {
    using Bus = arm::Bus;
    write<u8, Bus::Data>(0x04000247, 0x03); // wramcnt
    write<u8, Bus::Data>(0x04000300, 0x01); // postflg (arm9)
    write<u16, Bus::Data>(0x04000304, 0x0001); // powcnt1
    write<u32, Bus::Data>(0x027ff800, 0x00001fc2); // chip id 1
    write<u32, Bus::Data>(0x027ff804, 0x00001fc2); // chip id 2
    write<u16, Bus::Data>(0x027ff850, 0x5835); // arm7 bios crc
    write<u16, Bus::Data>(0x027ff880, 0x0007); // message from arm9 to arm7
    write<u16, Bus::Data>(0x027ff884, 0x0006); // arm7 boot task
    write<u32, Bus::Data>(0x027ffc00, 0x00001fc2); // copy of chip id 1
    write<u32, Bus::Data>(0x027ffc04, 0x00001fc2); // copy of chip id 2
    write<u16, Bus::Data>(0x027ffc10, 0x5835); // copy of arm7 bios crc
    write<u16, Bus::Data>(0x027ffc40, 0x0001); // boot indicator
}

} // namespace core::nds