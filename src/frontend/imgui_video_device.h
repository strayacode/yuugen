#pragma once

#include <SDL_opengl.h>
#include <array>
#include "common/types.h"
#include "common/video_device.h"

class ImGuiVideoDevice : public common::VideoDevice {
public:
    ImGuiVideoDevice();

    enum class Filter {
        Nearest,
        Linear,
    };

    void update_texture() override;
    void destroy() override;
    void configure(int width, int height, Filter filter);

    GLuint get_texture() { return texture; }
    u32* get_framebuffer() { return framebuffer.data(); }

private:
    std::array<u32, 256 * 192> framebuffer;
    GLuint texture;
    int width;
    int height;
};