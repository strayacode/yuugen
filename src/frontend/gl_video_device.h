#pragma once

#include <SDL_opengl.h>
#include "common/types.h"

class GLVideoDevice {
public:
    void initialise(int width, int height, bool nearest_filtering = true);
    void render(const u32* pointer);
    void destroy();

    GLuint get_texture() { return texture; }

private:
    GLuint texture;
    int width;
    int height;
};