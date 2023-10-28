#include "arm/jit/backend/a64/assembler.h"

namespace arm {

void A64Assembler::ret() {

}

void A64Assembler::emit(u32 data) {
    code.push_back(data);
}

} // namespace arm