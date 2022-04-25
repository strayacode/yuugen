#pragma once

#include <SDL2/SDL.h>
#include "AudioCommon/AudioInterface.h"

class SDLAudioInterface : public AudioInterface {
public:
    void Open(void* userdata, int sample_rate, int buffer_size, Callback callback) override;
    void Close() override;
    void SetState(AudioState state) override;

    SDL_AudioDeviceID device;
    SDL_AudioSpec spec;

    // TODO: add system spec as well to check for incompatibilities
};