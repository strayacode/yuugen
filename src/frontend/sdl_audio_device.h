#pragma once

#include <SDL.h>
#include "common/audio_device.h"

class SDLAudioDevice : public common::AudioDevice {
public:
    void configure(void* userdata, int sample_rate, int buffer_size, common::AudioCallback callback) override;
    void open() override;
    void close() override;
    void set_state(common::AudioState state) override;

private:
    common::AudioState audio_state = common::AudioState::Idle;
    void* userdata = nullptr;
    int sample_rate = 0;
    int buffer_size = 0;
    common::AudioCallback callback;
    SDL_AudioDeviceID device;
    SDL_AudioSpec spec;
};