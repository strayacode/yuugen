#include "common/logger.h"
#include "arm/interpreter/interpreter.h"
#include "arm/jit/jit.h"
#include "arm/jit/backend/ir_interpreter/ir_interpreter.h"
#include "gba/system.h"

namespace gba {

System::System() : memory(*this), ppu(*this), irq(cpu), timers(scheduler, irq), dma(scheduler, memory, irq) {
    arm::Config config;
    config.block_size = 1;
    config.backend_type = arm::BackendType::Interpreter;
    config.optimisations = false;
    configure_cpu_backend(config);
}

System::~System() {
    stop();
}

void System::System::reset() {
    scheduler.reset();
    cp14.reset();
    cpu->reset();
    cartridge.reset();
    cartridge.load(config.game_path);
    memory.reset();
    ppu.reset();
    irq.reset();
    input.reset();
    timers.reset();
    dma.reset();
    apu.reset();

    if (config.boot_mode == common::BootMode::Fast) {
        skip_bios();
    }
}

void System::run_frame() {
    auto frame_end = scheduler.get_current_time() + 280896;
    while (scheduler.get_current_time() < frame_end) {
        auto cycles = scheduler.get_event_time() - scheduler.get_current_time();

        if (!cpu->is_halted()) {
            cycles = std::min(static_cast<u64>(16), cycles);
        }

        cpu->run(cycles);
        scheduler.tick(cycles);
        scheduler.run();
    }
}

void System::set_audio_device(std::shared_ptr<common::AudioDevice> audio_device) {
    this->audio_device = audio_device;
}

void System::configure_cpu_backend(arm::Config config) {
    switch (config.backend_type) {
    case arm::BackendType::Interpreter:
        cpu = std::make_unique<arm::Interpreter>(arm::Arch::ARMv4, memory, cp14);
        break;
    case arm::BackendType::IRInterpreter:
        cpu = std::make_unique<arm::Jit>(arm::Arch::ARMv4, memory, cp14, config);
        break;
    case arm::BackendType::Jit:
        cpu = std::make_unique<arm::Jit>(arm::Arch::ARMv4, memory, cp14, config);
        break;
    }
}

std::vector<u32*> System::fetch_framebuffers() {
    return {ppu.fetch_framebuffer()};
}

void System::skip_bios() {
    // enter system mode
    auto cpsr = cpu->get_cpsr();
    cpsr.mode = arm::Mode::SYS;
    cpu->set_cpsr(cpsr);

    cpu->set_gpr(arm::GPR::SP, 0x03007f00);
    cpu->set_gpr(arm::GPR::SP, arm::Mode::IRQ, 0x03007fa0);
    cpu->set_gpr(arm::GPR::SP, arm::Mode::SVC, 0x03007fe0);
    cpu->set_gpr(arm::GPR::PC, 0x08000000);
}

} // namespace gba