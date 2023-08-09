#include "frontend/sdl_audio_device.h"

void SDLAudioDevice::configure(void* userdata, int sample_rate, int buffer_size, common::AudioCallback callback) {
    this->userdata = userdata;
    this->sample_rate = sample_rate;
    this->buffer_size = buffer_size;
    this->callback = callback;
}

void SDLAudioDevice::open() {
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

void SDLAudioDevice::close() {
    SDL_CloseAudioDevice(device);
}

void SDLAudioDevice::set_state(common::AudioState state) {
    switch (state) {
    case common::AudioState::Playing:
        if (audio_state == common::AudioState::Idle) {
            open();
        }

        SDL_PauseAudioDevice(device, false);
        break;
    case common::AudioState::Paused:
        SDL_PauseAudioDevice(device, true);
        break;
    case common::AudioState::Idle:
        close();
    }

    audio_state = state;
}