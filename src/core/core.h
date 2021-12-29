#pragma once

#include <functional>
#include <string>
#include <memory>
#include "yuugen_common/emu_thread.h"
#include "audio_common/audio_interface.h"
#include "audio_common/sdl/audio_interface.h"
#include "core/system.h"
#include "core/gba/gba.h"
#include "core/nds/nds.h"

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
    void DetectSystemType(std::string path);
    
    std::unique_ptr<System> system;
    std::shared_ptr<AudioInterface> audio_interface;
    
private:
    BootMode boot_mode = BootMode::Direct;
    State state = State::Idle;
    EmuThread emu_thread;
};