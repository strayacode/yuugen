#include <core/core.h>

Core::Core(UpdateFunction update_fps) : 
    emu_thread([this]() {
        RunFrame();
    }, update_fps) {
        
}

void Core::Initialise() {
    
}

void Core::Start() {
    system.Reset();

    if (boot_mode == BootMode::Firmware) {
        system.FirmwareBoot();
    } else {
        system.DirectBoot();
    }
}

void Core::Shutdown() {
    emu_thread.Stop();

    // later do save to config file and stuff
}

void Core::RunFrame() {
    system.RunFrame();
}

void Core::SetBootMode(BootMode new_mode) {
    boot_mode = new_mode;
}

BootMode Core::GetBootMode() {
    return boot_mode;
}

void Core::SetState(State new_state) {
    switch (new_state) {
    case State::Running:
        if (state == State::Idle) {
            Start();
        }

        emu_thread.Start();
        break;
    case State::Paused:
    case State::Idle:
        emu_thread.Stop();
        break;
    }

    state = new_state;
}

auto Core::GetState() -> State {
    return state;
}

void Core::SetRomPath(std::string path) {
    system.SetRomPath(path);
}

void Core::ToggleFramelimiter() {
    emu_thread.ToggleFramelimiter();
}

void Core::SetAudioInterface(AudioInterface& interface) {
    system.spu.SetAudioInterface(interface);
}

