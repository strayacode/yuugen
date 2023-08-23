#pragma once

#include "common/types.h"

namespace common {

enum class AudioState {
    Playing,
    Paused,
    Idle,
};

using AudioCallback = void (*)(void* userdata, s16* stream, int len);

class AudioDevice {
public:
    virtual ~AudioDevice() = default;

    virtual void configure(void* userdata, int sample_rate, int buffer_size, AudioCallback callback) = 0;
    virtual void open() = 0;
    virtual void close() = 0;
    virtual void set_state(AudioState state) = 0;
};

} // namespace common