#pragma once

#include <thread>
#include <chrono>
#include <ratio>
#include <stdio.h>
#include <core/core.h>
#include <functional>

struct EmuThread {
    EmuThread(Core& core, std::function<void(int fps)> update_fps);
    ~EmuThread();
    void Start();
    void Run();
    void Stop();
    auto IsActive() -> bool;
    auto GetFPS() -> int;
    void ToggleFramelimiter();

    Core& core;

    bool running = false;
    bool framelimiter = false;

    int frames = 0;

    std::thread thread;

    using frame = std::chrono::duration<int, std::ratio<1, 60>>;

    std::function<void(int fps)> update_fps;
};