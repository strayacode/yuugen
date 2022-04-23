#include "VideoCommon/GPU.h"
#include "VideoBackends2D/Software/SoftwareRenderer2D.h"

SoftwareRenderer2D::SoftwareRenderer2D(GPU& gpu, Engine engine) : Renderer2D(gpu, engine) {}

void SoftwareRenderer2D::render_scanline(int line) {

}