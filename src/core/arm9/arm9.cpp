#include "common/logger.h"
#include "arm/interpreter/interpreter.h"
#include "arm/jit/jit.h"
#include "arm/jit/backend/ir_interpreter/ir_interpreter.h"
#include "core/arm9/arm9.h"
#include "core/system.h"

namespace core {

ARM9::ARM9(System& system) : system(system), memory(system), coprocessor(cpu, memory), irq(cpu) {}

void ARM9::reset() {
    memory.reset();
    irq.reset();
    cpu->reset();
}

void ARM9::run(int cycles) {
    cpu->run(cycles);
}

void ARM9::select_backend(arm::BackendType backend) {
    switch (backend) {
    case arm::BackendType::Interpreter:
        cpu = std::make_unique<arm::Interpreter>(arm::Arch::ARMv5, memory, coprocessor);
        break;
    case arm::BackendType::IRInterpreter:
        cpu = std::make_unique<arm::Jit>(arm::Arch::ARMv5, memory, coprocessor, arm::BackendType::IRInterpreter);
        break;
    default:
        logger.error("ARM9: unknown backend");
    }
}

void ARM9::direct_boot() {
    using Bus = arm::Bus;
    memory.write<u8, Bus::Data>(0x04000300, 0x01); // postflg (arm9)
    memory.write<u16, Bus::Data>(0x04000304, 0x0001); // powcnt1
    memory.write<u32, Bus::Data>(0x027ff800, 0x00001fc2); // chip id 1
    memory.write<u32, Bus::Data>(0x027ff804, 0x00001fc2); // chip id 2
    memory.write<u16, Bus::Data>(0x027ff850, 0x5835); // arm7 bios crc
    memory.write<u16, Bus::Data>(0x027ff880, 0x0007); // message from arm9 to arm7
    memory.write<u16, Bus::Data>(0x027ff884, 0x0006); // arm7 boot task
    memory.write<u32, Bus::Data>(0x027ffc00, 0x00001fc2); // copy of chip id 1
    memory.write<u32, Bus::Data>(0x027ffc04, 0x00001fc2); // copy of chip id 2
    memory.write<u16, Bus::Data>(0x027ffc10, 0x5835); // copy of arm7 bios crc
    memory.write<u16, Bus::Data>(0x027ffc40, 0x0001); // boot indicator

    coprocessor.write(1, 0, 0, 0x0005707d);
    coprocessor.write(9, 1, 0, 0x0300000a);
    coprocessor.write(9, 1, 1, 0x00000020);

    // enter system mode
    auto cpsr = cpu->get_cpsr();
    cpsr.mode = arm::Mode::SYS;
    cpu->set_cpsr(cpsr);

    cpu->set_gpr(arm::GPR::R12, system.cartridge.get_arm9_entrypoint());
    cpu->set_gpr(arm::GPR::SP, 0x03002f7c);
    cpu->set_gpr(arm::GPR::SP, arm::Mode::IRQ, 0x03003f80);
    cpu->set_gpr(arm::GPR::SP, arm::Mode::SVC, 0x03003fc0);
    cpu->set_gpr(arm::GPR::LR, system.cartridge.get_arm9_entrypoint());
    cpu->set_gpr(arm::GPR::PC, system.cartridge.get_arm9_entrypoint());
}

bool ARM9::is_halted() {
    return cpu->is_halted();
}

} // namespace core