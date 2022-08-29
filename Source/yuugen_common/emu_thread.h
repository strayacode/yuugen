#pragma once

#include <thread>
#include <chrono>
#include <ratio>
#include <stdio.h>
#include <functional>

using RunFunction = std::function<void()>;
using UpdateFunction = std::function<void(float fps)>;

class EmuThread {
public:
    EmuThread(RunFunction run_frame, UpdateFunction update_fps);
    ~EmuThread();
    void Start();
    void Reset();
    void Run();
    void Stop();
    bool IsActive();
    int GetFPS();
    void toggle_framelimiter();
    bool framelimiter_enabled() { return framelimiter; }

    std::thread thread;

    using frame = std::chrono::duration<int, std::ratio<1, 60>>;

    RunFunction run_frame;
    UpdateFunction update_fps;

private:
    int frames = 0;
    bool running = false;
    bool framelimiter = false;

    static constexpr int update_interval = 1000;
};
