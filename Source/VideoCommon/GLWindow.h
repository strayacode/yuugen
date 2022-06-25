#pragma once

#include <GL/glew.h>
#include "Common/Types.h"

class GLWindow {
public:
    void initialise(int width, int height, bool nearest_filtering = true);
    void render(const u32* pointer);
    void destroy();

    GLuint get_texture() { return texture; }

private:
    GLuint texture;
    int width = 0;
    int height = 0;
};