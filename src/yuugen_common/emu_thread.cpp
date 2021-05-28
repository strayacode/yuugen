#include <yuugen_common/emu_thread.h>

EmuThread::EmuThread(Core& core) : core(core) {

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
    while (running) {
        core.RunFrame();
        frames++;

        if (framelimiter) {
            // block the execution of the emulator thread until 1 / 60 of a second has passed
            std::this_thread::sleep_until(frame_end);
        }

        frame_end += frame{1};
    }
}
 
void EmuThread::Stop() {
    // allow the emulator to first complete a frame, and then pause emulation
    running = false;

    thread.join();
}

auto EmuThread::IsActive() -> bool {
    return running;
}