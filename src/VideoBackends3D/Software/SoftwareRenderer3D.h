#pragma once

#include "VideoCommon/Renderer3D.h"

class SoftwareRenderer3D : public Renderer3D {
public:
    void render() override;
private:
};