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

} // namespace core::nds