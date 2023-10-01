#include "common/logger.h"
#include "arm/interpreter/interpreter.h"
#include "arm/jit/jit.h"
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

void ARM7::select_backend(arm::BackendType backend, bool optimise) {
    switch (backend) {
    case arm::BackendType::Interpreter:
        cpu = std::make_unique<arm::Interpreter>(arm::Arch::ARMv4, memory, coprocessor);
        break;
    case arm::BackendType::IRInterpreter:
        cpu = std::make_unique<arm::Jit>(arm::Arch::ARMv4, memory, coprocessor, arm::BackendType::IRInterpreter, optimise);
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

    // enter system mode
    auto cpsr = cpu->get_cpsr();
    cpsr.mode = arm::Mode::SYS;
    cpu->set_cpsr(cpsr);

    cpu->set_gpr(arm::GPR::R12, system.cartridge.get_arm7_entrypoint());
    cpu->set_gpr(arm::GPR::SP, 0x0380fd80);
    cpu->set_gpr(arm::GPR::SP, arm::Mode::IRQ, 0x0380ff80);
    cpu->set_gpr(arm::GPR::SP, arm::Mode::SVC, 0x0380ffc0);
    cpu->set_gpr(arm::GPR::LR, system.cartridge.get_arm7_entrypoint());
    cpu->set_gpr(arm::GPR::PC, system.cartridge.get_arm7_entrypoint());
}

bool ARM7::is_halted() {
    return cpu->is_halted();
}

} // namespace core