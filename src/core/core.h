#pragma once

#include <core/hw/hw.h>
#include <yuugen_common/emu_thread.h>
#include <audio_common/audio_interface.h>
#include <core/config.h>
#include <functional>
#include <string>

enum class BootMode {
    Direct,
    Firmware,
};

enum class State {
    Running,
    Paused,
    Idle,
};

using UpdateFunction = std::function<void(float fps)>;

class Core {
public:
    Core(UpdateFunction update_fps);
    void Initialise();
    void Start();
    void Reset();
    void Stop();
    void Shutdown();

    void RunFrame();
    void SetBootMode(BootMode new_mode);
    BootMode GetBootMode();
    void SetState(State new_state);
    auto GetState() -> State;
    void SetRomPath(std::string path);
    void ToggleFramelimiter();
    void SetAudioInterface(AudioInterface& interface);

    HW hw;

    Config config;
    
private:
    // if not configured yet just direct boot
    BootMode boot_mode = BootMode::Direct;

    // initally the core should be idle
    State state = State::Idle;

    EmuThread emu_thread;
};