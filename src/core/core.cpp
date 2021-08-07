#include <core/core.h>

// we don't have much use for Core rn but later it will 
// have more functionality
Core::Core(UpdateFunction update_fps) : 
    emu_thread([this]() {
        RunFrame();
    }, update_fps) {

}

void Core::Initialise() {
    
}

void Core::Start() {
    hw.Reset();

    if (boot_mode == BootMode::Firmware) {
        hw.FirmwareBoot();
    } else {
        hw.DirectBoot();
    }
}

void Core::Shutdown() {
    emu_thread.Stop();

    // later do save to config file and stuff
}

void Core::RunFrame() {
    hw.RunFrame();
}

void Core::SetBootMode(BootMode new_mode) {
    boot_mode = new_mode;
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
    hw.SetRomPath(path);
}