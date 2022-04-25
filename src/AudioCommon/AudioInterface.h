#pragma once

#include "Common/Types.h"

enum class AudioState {
    Playing,
    Paused,
};

typedef void (*Callback)(void* userdata, s16* stream, int len);

class AudioInterface {
public:
    virtual void Open(void* userdata, int sample_rate, int buffer_size, Callback callback) = 0;
    virtual void Close() = 0;
    virtual void SetState(AudioState state) = 0;

    AudioState audio_state;
};