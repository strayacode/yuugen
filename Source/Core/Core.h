#pragma once

#include <functional>
#include <string>
#include <memory>
#include <yuugen_common/emu_thread.h>
#include "AudioCommon/AudioInterface.h"
#include "AudioCommon/SDLAudioInterface.h"
#include "Core/System.h"

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
    void Start();
    void Reset();
    void Stop();
    void Shutdown();

    void RunFrame();
    void SetBootMode(BootMode new_mode);
    BootMode GetBootMode();
    void SetState(State new_state);
    State GetState();
    void BootGame(std::string path);
    void BootFirmware();
    void ToggleFramelimiter();
    void SetGamePath(std::string path);
    bool framelimiter_enabled() { return emu_thread.framelimiter_enabled(); }

    System system;
    std::shared_ptr<AudioInterface> audio_interface;
    
private:
    BootMode boot_mode = BootMode::Direct;
    State state = State::Idle;
    EmuThread emu_thread;
};