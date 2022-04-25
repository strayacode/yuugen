#include "Common/Settings.h"
#include "Core/core.h"

Core::Core(UpdateFunction update_fps) : 
    emu_thread([this]() {
        RunFrame();
    }, update_fps) {
    audio_interface = std::make_shared<SDLAudioInterface>();
    system.spu.SetAudioInterface(audio_interface);
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

        audio_interface->SetState(AudioState::Playing);
        emu_thread.Start();

        if (Settings::Get().threaded_2d) {
            system.gpu.start_render_thread();
        }
        break;
    case State::Paused:
    case State::Idle:
        audio_interface->SetState(AudioState::Paused);
        emu_thread.Stop();

        if (Settings::Get().threaded_2d) {
            system.gpu.stop_render_thread();
        }
        break;
    }

    state = new_state;
}

State Core::GetState() {
    return state;
}

void Core::BootGame(std::string path) {
    SetState(State::Idle);
    SetGamePath(path);
    SetState(State::Running);
}

void Core::BootFirmware() {
    BootMode old_boot_mode = GetBootMode();

    SetState(State::Idle);
    SetGamePath("");
    SetBootMode(BootMode::Firmware);
    SetState(State::Running);

    // make sure to go back to the previously configured boot mode
    SetBootMode(old_boot_mode);
}

void Core::ToggleFramelimiter() {
    emu_thread.ToggleFramelimiter();
}

void Core::SetGamePath(std::string path) {
    system.SetGamePath(path);
}
