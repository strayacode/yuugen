#pragma once

#include <common/types.h>

class AudioInterface {
    virtual void Open(void* userdata, int sample_rate, int buffer_size, Callback callback) = 0;
    virtual void Close() = 0;

    typedef void (*Callback)(void* userdata, s16* stream, int len);
};