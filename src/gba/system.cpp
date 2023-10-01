#include "common/logger.h"
#include "arm/interpreter/interpreter.h"
#include "gba/system.h"

namespace gba {

System::System() : memory(*this) {
    cpu = std::make_unique<arm::Interpreter>(arm::Arch::ARMv4, memory, cp14);
}

System::~System() {
    stop();
}

void System::System::reset() {
    memory.reset();
    cp14.reset();
    cpu->reset();

    frames = 0;

    if (config.boot_mode == common::BootMode::Fast) {
        skip_bios();
    }
}

void System::run_frame() {
    logger.todo("handle run_frame");
}

void System::set_audio_device(std::shared_ptr<common::AudioDevice> audio_device) {
    logger.warn("gba::System: set_audio_device");
}

std::vector<u32*> System::fetch_framebuffers() {
    logger.todo("handle fetch_framebuffers");
    return {nullptr};
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