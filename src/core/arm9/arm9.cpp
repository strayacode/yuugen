#include "common/logger.h"
#include "arm/interpreter/interpreter.h"
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

void ARM9::select_backend(arm::Backend backend) {
    switch (backend) {
    case arm::Backend::Interpreter:
        cpu = std::make_unique<arm::Interpreter>(arm::Arch::ARMv5, memory, coprocessor);
        break; 
    default:
        logger.error("ARM9: unknown backend");
    }
}

void ARM9::direct_boot() {
    using Bus = arm::Bus;
    memory.write<u8, Bus::Data>(0x04000247, 0x03); // wramcnt
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

    cpu->jump_to(system.cartridge.get_arm9_entrypoint());

    auto& state = cpu->get_state();
    state.gpr[12] = state.gpr[14] = state.gpr[15];
    state.gpr[13] = 0x03002f7c;
    state.gpr_banked[arm::Bank::IRQ][5] = 0x03003f80;
    state.gpr_banked[arm::Bank::SVC][5] = 0x03003fc0;

    // enter system mode
    state.cpsr.data = 0xdf;
    cpu->set_mode(arm::Mode::SYS);
}

} // namespace core