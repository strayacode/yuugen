#include "yuugen_common/emu_thread.h"

EmuThread::EmuThread(RunFunction run_frame, UpdateFunction update_fps) : run_frame(run_frame), update_fps(update_fps) {

}

EmuThread::~EmuThread() {
    Stop();
}

void EmuThread::Start() {
    running = true;

    thread = std::thread{[this]() {
        Run();
    }};
}

void EmuThread::Reset() {
    frames = 0;
}

void EmuThread::Run() {
    auto frame_end = std::chrono::system_clock::now() + frame{1};
    auto fps_update = std::chrono::system_clock::now();
    while (running) {
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
        }

        frame_end += frame{1};
    }
}
 
void EmuThread::Stop() {
    if (!running) {
        return;
    }

    // allow the emulator to first complete a frame, and then pause emulation
    running = false;

    thread.join();
}

bool EmuThread::IsActive() {
    return running;
}

int EmuThread::GetFPS() {
    return frames;
}

void EmuThread::toggle_framelimiter() {
    framelimiter = !framelimiter;
}
