#include "nds/arm7/coprocessor.h"

namespace nds {

void ARM7Coprocessor::reset() {

}

u32 ARM7Coprocessor::read(u32 cn, u32 cm, u32 cp) {
    return 0;
}

void ARM7Coprocessor::write(u32 cn, u32 cm, u32 cp, u32 value) {

}

u32 ARM7Coprocessor::get_exception_base() {
    return 0;
}

} // namespace nds