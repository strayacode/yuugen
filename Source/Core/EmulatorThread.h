#pragma once

#include <thread>
#include <chrono>
#include <ratio>
#include <functional>

using RunFunction = std::function<void()>;
using UpdateFunction = std::function<void(float fps)>;

class System;

class EmulatorThread {
public:
    EmulatorThread(System& system, RunFunction run_frame, UpdateFunction update_fps);
    ~EmulatorThread();
    void Start();
    void Reset();
    void Run();
    void Stop();
    bool IsActive();
    int GetFPS();
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
