#include "AudioCommon/SDLAudioInterface.h"

void SDLAudioInterface::open() {
    SDL_Init(SDL_INIT_AUDIO);

    SDL_memset(&spec, 0, sizeof(SDL_AudioSpec));

    spec.freq = sample_rate;
    spec.format = AUDIO_S16;
    spec.channels = 2;
    spec.samples = buffer_size;
    spec.callback = (SDL_AudioCallback)callback;
    spec.userdata = userdata;

    // TODO: look at other options for SDL_OpenAudioDevice
    device = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
}

void SDLAudioInterface::close() {
    SDL_CloseAudioDevice(device);
}

void SDLAudioInterface::SetState(AudioState state) {
    switch (state) {
    case AudioState::Playing:
        if (audio_state == AudioState::Idle) {
            open();
        }

        SDL_PauseAudioDevice(device, false);
        break;
    case AudioState::Paused:
        SDL_PauseAudioDevice(device, true);
        break;
    case AudioState::Idle:
        close();
    }

    audio_state = state;
}