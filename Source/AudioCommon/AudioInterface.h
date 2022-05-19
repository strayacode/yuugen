#pragma once

#include "Common/Types.h"

enum class AudioState {
    Playing,
    Paused,
    Idle,
};

typedef void (*Callback)(void* userdata, s16* stream, int len);

class AudioInterface {
public:
    void configure(void* userdata, int sample_rate, int buffer_size, Callback callback) {
        this->userdata = userdata;
        this->sample_rate = sample_rate;
        this->buffer_size = buffer_size;
        this->callback = callback;
    }

    virtual void open() = 0;
    virtual void close() = 0;
    virtual void SetState(AudioState state) = 0;

    AudioState audio_state = AudioState::Idle;
    void* userdata = nullptr;
    int sample_rate = 0;
    int buffer_size = 0;
    Callback callback;
};