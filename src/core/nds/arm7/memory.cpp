#include "core/nds/arm7/arm7.h"
#include "core/nds/system.h"

namespace core::nds {

ARM7Memory::ARM7Memory(System& system) : system(system) {}

u8 ARM7Memory::system_read_byte(u32 addr) {
    return 0;
}

u16 ARM7Memory::system_read_half(u32 addr) {
    return 0;
}

u32 ARM7Memory::system_read_word(u32 addr) {
    return 0;
}

void ARM7Memory::system_write_byte(u32 addr, u8 value) {

}

void ARM7Memory::system_write_half(u32 addr, u16 value) {

}

void ARM7Memory::system_write_word(u32 addr, u32 value) {

}

} // namespace core::nds