#pragma once

#include <thread>
#include <array>
#include <chrono>
#include <ratio>
#include <memory>
#include <vector>
#include "common/types.h"
#include "common/config.h"
#include "common/audio_device.h"
#include "common/callback.h"

namespace common {

class System {
public:
    virtual ~System() = default;
    virtual void reset() = 0;
    virtual void run_frame() = 0;
    virtual void set_audio_device(std::shared_ptr<common::AudioDevice> audio_device) = 0;
    virtual std::vector<u32*> fetch_framebuffers() = 0;

    void set_game_path(const std::string& game_path);
    void set_boot_mode(BootMode boot_mode);

    using UpdateCallback = common::Callback<void(f32)>;

    void set_update_callback(UpdateCallback update_callback) { this->update_callback = update_callback; }

    enum class State {
        Running,
        Paused,
        Idle,
    };

    State get_state() { return state; }
    bool is_running() { return state == State::Running; }

    void set_state(State new_state);
    void toggle_framelimiter();
    void stop();
    void pause();
    void resume();

    Config config;
    std::shared_ptr<common::AudioDevice> audio_device;
    int frames{0};
    bool framelimiter{true};

private:
    void run_thread();
    void start();

    std::thread thread;

    enum class ThreadState {
        Running,
        Idle,
    };

    ThreadState thread_state = ThreadState::Idle;
    State state = State::Idle;

    using Frame = std::chrono::duration<int, std::ratio<1, 60>>;

    UpdateCallback update_callback;

    static constexpr int FPS_UPDATE_INTERVAL = 500;
};

} // namespace common