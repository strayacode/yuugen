#include "common/logger.h"
#include "core/arm/interpreter/interpreter.h"
#include "core/nds/arm7/arm7.h"
#include "core/nds/system.h"

namespace core::nds {

ARM7::ARM7(System& system) : memory(system) {}

void ARM7::run(int cycles) {
    cpu->run(cycles);
}

void ARM7::select_backend(arm::Backend backend) {
    switch (backend) {
    case arm::Backend::Interpreter:
        cpu = std::make_unique<arm::Interpreter>();
        break; 
    default:
        logger.error("ARM7: unknown backend");
    }
}

void ARM7::direct_boot() {
    memory.direct_boot();
}

} // namespace core::nds