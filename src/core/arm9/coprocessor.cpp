#include "core/arm9/coprocessor.h"
#include "core/arm9/memory.h"

namespace core {

ARM9Coprocessor::ARM9Coprocessor(ARM9Memory& memory) : memory(memory) {}

u32 ARM9Coprocessor::read(u32 cn, u32 cm, u32 cp) {
    return 0;
}

void ARM9Coprocessor::write(u32 cn, u32 cm, u32 cp, u32 value) {

}

u32 ARM9Coprocessor::get_exception_base() {
    return 0;
}

} // namespace core