#pragma once

#include <SDL2/SDL.h>
#include "AudioCommon/AudioInterface.h"

class SDLAudioInterface : public AudioInterface {
public:
    void open() override;
    void close() override;
    void SetState(AudioState state) override;

    SDL_AudioDeviceID device;
    SDL_AudioSpec spec;

    // TODO: add system spec as well to check for incompatibilities
};