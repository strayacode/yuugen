#include <yuugen_common/emu_thread.h>

EmuThread::EmuThread(Core& core) : core(core) {
    printf("pog\n");
    running = true;
}

void EmuThread::Start() {
    // thread = std::thread{[this]() {
    //     Run();
    // }};
}

void EmuThread::Pause() {

}

void EmuThread::Stop() {

}

void EmuThread::Run() {
    while (running) {

    }
}