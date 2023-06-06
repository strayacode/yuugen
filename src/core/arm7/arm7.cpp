#include "common/logger.h"
#include "arm/interpreter/interpreter.h"
#include "core/arm7/arm7.h"
#include "core/system.h"

namespace core {

ARM7::ARM7(System& system) : system(system), memory(system), irq(cpu) {}

void ARM7::reset() {
    memory.reset();
    irq.reset();
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

    auto& state = cpu->get_state();
    state.gpr[12] = state.gpr[14] = state.gpr[15] = system.cartridge.get_arm7_entrypoint();
    state.gpr[13] = 0x0380fd80;
    state.gpr_banked[arm::Bank::IRQ][5] = 0x0380ff80;
    state.gpr_banked[arm::Bank::SVC][5] = 0x0380ffc0;

    // enter system mode
    state.cpsr.data = 0xdf;
    cpu->set_mode(arm::Mode::SYS);
    cpu->flush_pipeline();
}

} // namespace core