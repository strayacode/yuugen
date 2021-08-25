#include "audio_interface.h"

void SDLAudioInterface::Open(void* userdata, int sample_rate, int buffer_size, Callback callback) {
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

void SDLAudioInterface::Close() {
    SDL_CloseAudioDevice(device);
}

void SDLAudioInterface::SetState(AudioState state) {
    audio_state = state;

    SDL_PauseAudioDevice(device, state == AudioState::Paused);
}