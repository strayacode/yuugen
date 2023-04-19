#include "core/nds/arm9/coprocessor.h"
#include "core/nds/arm9/memory.h"

namespace core::nds {

ARM9Coprocessor::ARM9Coprocessor(ARM9Memory& memory) : memory(memory) {}

u32 ARM9Coprocessor::read(u32 cn, u32 cm, u32 cp) {
    return 0;
}

void ARM9Coprocessor::write(u32 cn, u32 cm, u32 cp, u32 value) {

}

} // namespace core::nds