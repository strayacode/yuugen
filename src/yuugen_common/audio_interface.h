#pragma once

#include <common/types.h>

typedef void (*Callback)(void* userdata, s16* stream, int len);

class AudioInterface {
public:
    virtual void Open(void* userdata, int sample_rate, int buffer_size, Callback callback) = 0;
    virtual void Close() = 0;
};