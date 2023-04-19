#include "common/logger.h"
#include "core/arm/interpreter/interpreter.h"
#include "core/nds/arm7/arm7.h"
#include "core/nds/system.h"

namespace core::nds {

ARM7::ARM7(System& system) : memory(system) {}

void ARM7::reset() {
    cpu->reset();
}

void ARM7::run(int cycles) {
    cpu->run(cycles);
}

void ARM7::select_backend(arm::Backend backend) {
    switch (backend) {
    case arm::Backend::Interpreter:
        cpu = std::make_unique<arm::Interpreter>(arm::Arch::ARMv4, memory, coprocessor);
        break; 
    default:
        logger.error("ARM7: unknown backend");
    }
}

void ARM7::direct_boot() {
    using Bus = arm::Bus;
    memory.write<u16, Bus::Data>(0x04000134, 0x8000); // rcnt
    memory.write<u8, Bus::Data>(0x04000300, 0x01); // postflg (arm7)
    memory.write<u16, Bus::Data>(0x04000504, 0x0200); // soundbias


}

} // namespace core::nds