#include "Core/EmulatorThread.h"
#include "Core/System.h"

EmulatorThread::EmulatorThread(System& system, RunFunction run_frame, UpdateFunction update_fps) : m_system(system), run_frame(run_frame), update_fps(update_fps) {}

EmulatorThread::~EmulatorThread() {
    stop();
}

void EmulatorThread::start() {
    running = true;

    thread = std::thread{[this]() {
        run();
    }};
}

void EmulatorThread::reset() {
    frames = 0;
}

void EmulatorThread::run() {
    auto frame_end = std::chrono::system_clock::now() + frame{1};
    auto fps_update = std::chrono::system_clock::now();
    while (running) {
        if (m_system.state() == State::Running) {
            run_frame();
            frames++;

            if (std::chrono::system_clock::now() - fps_update >= std::chrono::milliseconds(update_interval)) {
                update_fps(frames * (1000.0f / update_interval));
                frames = 0;
                fps_update = std::chrono::system_clock::now();
            }

            if (framelimiter) {
                // block the execution of the emulator thread until 1 / 60 of a second has passed
                std::this_thread::sleep_until(frame_end);

                frame_end += frame{1};
            } else {
                frame_end = std::chrono::system_clock::now() + frame{1};
            }
        }
    }
}
 
void EmulatorThread::stop() {
    if (!running) {
        return;
    }

    // allow the emulator to first complete a frame, and then pause emulation
    running = false;

    thread.join();
}

void EmulatorThread::toggle_framelimiter() {
    framelimiter = !framelimiter;
}
