#pragma once

#include <thread>
#include <chrono>
#include <ratio>
#include "Common/Callback.h"

using RunFunction = Common::Callback<void()>;
using UpdateFunction = Common::Callback<void(float fps)>;

class System;

class EmulatorThread {
public:
    EmulatorThread(System& system, RunFunction run_frame, UpdateFunction update_fps);
    ~EmulatorThread();

    void start();
    void reset();
    void run();
    void stop();
    
    void toggle_framelimiter();
    bool framelimiter_enabled() { return framelimiter; }

private:
    using frame = std::chrono::duration<int, std::ratio<1, 60>>;

    int frames = 0;
    bool running = false;
    bool framelimiter = false;

    System& m_system;
    RunFunction run_frame;
    UpdateFunction update_fps;
    std::thread thread;

    static constexpr int update_interval = 1000;
};
