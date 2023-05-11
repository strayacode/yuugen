#pragma once

#include <SDL_opengl.h>
#include "common/types.h"
#include "common/video_device.h"

class ImGuiVideoDevice : public common::VideoDevice {
public:
    enum class Filter {
        Nearest,
        Linear,
    };

    void update_texture(u32* pointer) override;
    void destroy() override;
    void configure(int width, int height, Filter filter);

    GLuint get_texture() { return texture; }

private:
    GLuint texture;
    int width;
    int height;
};