#include "common/logger.h"
#include "core/arm/interpreter/interpreter.h"
#include "core/nds/arm9/arm9.h"
#include "core/nds/system.h"

namespace core::nds {

ARM9::ARM9(System& system) : memory(system) {}

void ARM9::run(int cycles) {
    cpu->run(cycles);
}

void ARM9::select_backend(arm::Backend backend) {
    switch (backend) {
    case arm::Backend::Interpreter:
        cpu = std::make_unique<arm::Interpreter>();
        break; 
    default:
        logger.error("ARM9: unknown backend");
    }
}

void ARM9::direct_boot() {
    memory.direct_boot();
}

} // namespace core::nds