#include <yuugen_common/emu_thread.h>

EmuThread::EmuThread(Core& core, std::function<void(int fps)> update_fps) : core(core), update_fps(update_fps) {

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

void EmuThread::Run() {
    auto frame_end = std::chrono::system_clock::now() + frame{1};
    auto fps_update = std::chrono::system_clock::now();
    while (running) {
        core.RunFrame();
        frames++;

        if (std::chrono::system_clock::now() - fps_update >= std::chrono::milliseconds(1000)) {
            update_fps(frames);
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

auto EmuThread::IsActive() -> bool {
    return running;
}

auto EmuThread::GetFPS() -> int {
    return frames;
}