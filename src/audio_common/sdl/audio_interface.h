#pragma once

#include <SDL2/SDL.h>
#include <audio_common/audio_interface.h>

class SDLAudioInterface : public AudioInterface {
public:
    void Open(void* userdata, int sample_rate, int buffer_size, Callback callback) override;
    void Close() override;

    SDL_AudioDeviceID device;
    SDL_AudioSpec spec;

    // TODO: add system spec as well to check for incompatibilities
};