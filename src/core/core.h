#pragma once

#include <core/hw/hw.h>
#include <yuugen_common/emu_thread.h>
#include <functional>
#include <string>

enum class BootMode {
    Firmware,
    Direct,
};

enum class State {
    Running,
    Paused,
    Idle,
};

using UpdateFunction = std::function<void(int fps)>;

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
    void SetState(State new_state);
    auto GetState() -> State;
    void SetRomPath(std::string path);

    HW hw;
    
private:
    BootMode boot_mode;

    // initally the core should be idle
    State state = State::Idle;

    EmuThread emu_thread;
};